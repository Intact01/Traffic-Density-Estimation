#include <bits/stdc++.h>

#include "utility.hpp"
#include "arg_parser.hpp"
#include "image_operations.hpp"
#include "density.hpp"
#include "choice.hpp"
#include "parameters.hpp"

int frameskip;
string videoPath;
string imagePath;
Parameters parameters;

cv::VideoCapture getImageStream(string videoPath) {
  cv::VideoCapture capture(videoPath);

  if (!capture.isOpened()) {
    // error in opening the video input
    cerr << "Unable to open file!" << endl;
  }
  return capture;
}

void make_csv(vector<double> queue_density_list,
              vector<double> moving_density_list) {
  string comma = ",";
  fstream fout;
  fout.open("output/density.csv", ios::out);
  fout << "Frame" << comma << "Queue Density" << comma << "Moving Static"
       << endl;

  for (int i = 0; i < queue_density_list.size(); i++) {
    fout << i << comma << queue_density_list[i] << comma
         << moving_density_list[i] << endl;
  }
  fout.close();
}

void start(vector_point source_pts = scr_pts) {
  cv::VideoCapture capture = getImageStream(videoPath);
  vector<double> queue_density_list, moving_density_list;

  initialize(capture.get(cv::CAP_PROP_FRAME_COUNT));

  parameters.initialize();
  //calc_density(queue_density_list, moving_density_list, capture, frameskip, source_pts);
  method2(queue_density_list, capture, source_pts);
  parameters.complete();

  cout << " queue density : " << queue_density_list.size() << endl;
  for(int i=0;i<10;i++){
    cout<<queue_density_list[i]<<endl;
  }

  // make_graph(queue_density_list, moving_density_list, imagePath, frameskip);
  double utility_queue = find_utility_qd(queue_density_list, 1);
  // double utility_moving = find_utility_qd(queue_density_list, frameskip);
  cout << utility_queue << endl;
  // make_csv(queue_density_list, moving_density_list);
}
bool hasEnding(std::string const &fileName, std::string const &extension) {
  if (fileName.length() >= extension.length()) {
    return (0 == fileName.compare(fileName.length() - extension.length(),
                                  extension.length(), extension));
  } else {
    return false;
  }
}
// main function
int main(int argc, char **argv) {
  frameskip = 5;
  videoPath = "input/trafficvideo.mp4";
  imagePath = "output/output.png";
  bool choose = false;
  parameters = Parameters();

  parse(argc, argv, imagePath, videoPath, frameskip, choose);

  std::ifstream file(videoPath);
  if (!file.is_open()) {
    std::cout << "File not found. Aborting" << std::endl;
    return -1;
  }
  if (!(hasEnding(videoPath, ".avi") || hasEnding(videoPath, ".mp4") ||
        hasEnding(videoPath, ".m4u") || hasEnding(videoPath, ".mkv"))) {
    std::cout << "Invalid File. Aborting" << std::endl;
    return -1;
  }

  if (choose) {
    getCustomPoints(start);
  } else {
    start();
  }

  cout << "time elapsed: " << parameters.get_time_elapsed() << endl;

  return 0;
}

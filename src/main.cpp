#include <bits/stdc++.h>

#include "arg_parser.hpp"
#include "choice.hpp"
#include "density.hpp"
#include "helpers.hpp"
#include "image_operations.hpp"
#include "parameters.hpp"
#include "utility.hpp"

int frameskip;
string videoPath;
string imagePath;
Parameters parameters;
Resolution resolution = {480, 360};
Density density;

int method = 4;
int num_threads = 5;

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

void jump(cv::VideoCapture &cap, int pos) {
  while (pos--) {
    cap.grab();
  }
}

void start(vector_point source_pts = scr_pts) {
  density.capture = getImageStream(videoPath);
  density.capture.set(cv::CAP_PROP_FORMAT, CV_32F);
  density.source_points = source_pts;

  initialize(density.capture.get(cv::CAP_PROP_FRAME_COUNT));

  parameters.initialize();

  switch (method) {
    case 1:
      density.method1();
      break;
    case 2:
      density.method2();
      break;
    case 3:
      density.method3();
      break;
    case 4:
      density.method4();
      break;
    case 5:
      density.method5();
      break;
    case 6:
      density.method0_md();
      break;
    case 0:
      density.method0_qd();
      break;
    default:
      return;
  }

  parameters.complete();

  if (method < 5) {
    double utility_queue =
        find_utility_qd(density.queue_density_list, frameskip);
    logger.log("Utility is: " + to_string(utility_queue));
    cout << utility_queue << endl;
  } else {
    double utility_moving =
        find_utility_md(density.moving_density_list, frameskip);
    logger.log("Utility is: " + to_string(utility_moving));
    cout << utility_moving << endl;
  }

  make_graph(density.queue_density_list, density.moving_density_list, imagePath,
             frameskip);
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
  frameskip = 1;
  videoPath = "input/trafficvideo.mp4";
  imagePath = "output/output.png";
  bool choose = false;
  bool verbose = false;

  parameters = Parameters();

  int res =
      parse(argc, argv, imagePath, videoPath, density.fast_forward, choose,
            method, density.num_threads, logger.enable, density.resolution);
  if (res != 0) return 1;
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

  cout << parameters.get_time_elapsed() << endl;
  return 0;
}

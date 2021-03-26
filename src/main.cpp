#include <bits/stdc++.h>

#include "arg_parser.hpp"
#include "choice.hpp"
#include "density.hpp"
#include "image_operations.hpp"
#include "parameters.hpp"
#include "utility.hpp"

int frameskip;
string videoPath;
string imagePath;
Parameters parameters;

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

void start(vector_point source_pts = scr_pts) {
  vector<cv::VideoCapture> captures;
  captures.push_back(getImageStream(videoPath));

  // cout << "size is " << captures.size() << endl;
  // cv::VideoCapture cp = captures[1];
  // cv::Mat frame_temp;
  // cp >> frame_temp;
  // imshow("temp", frame_temp);
  // cv::waitKey(0);

  for (int i = 0; i < num_threads; i++) {
    cv::VideoCapture cap = getImageStream(videoPath);
    int num_frames = captures[0].get(cv::CAP_PROP_FRAME_COUNT);
    int frames_per_thread = ceil((num_frames - 500) / num_threads);
    cap.set(cv::CAP_PROP_POS_FRAMES, 500 + i * frames_per_thread);
    cap.set(cv::CAP_PROP_FORMAT, CV_32F);
    captures.push_back(cap);
  }

  vector<double> queue_density_list, moving_density_list;

  initialize(captures[0].get(cv::CAP_PROP_FRAME_COUNT));

  parameters.initialize();
  // calc_density(queue_density_list, moving_density_list, capture, frameskip,
  // source_pts);
  switch (method) {
    case 1:
      method1(queue_density_list, captures[0], source_pts);
    case 2:
      method2(queue_density_list, captures[0], source_pts);
    case 3:
      method3(queue_density_list, captures, source_pts, num_threads);
    case 4:
      method4(queue_density_list, captures, source_pts, num_threads);
    case 5:
      method5(moving_density_list, captures[0], source_pts);
    default:
      method0(queue_density_list, captures[0], source_pts);
  }
  // method4(queue_density_list, captures, source_pts, num_threads);
  // method0(queue_density_list, capture, source_pts);

  parameters.complete();

  cout << " queue density : " << queue_density_list.size() << endl;
  for (int i = 0; i < 10; i++) {
    cout << queue_density_list[i] << endl;
  }

  make_graph(queue_density_list, moving_density_list, imagePath, frameskip);
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

  parse(argc, argv, imagePath, videoPath, frameskip, choose, method,
        num_threads);

  cout << num_threads << " " << method << endl;

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

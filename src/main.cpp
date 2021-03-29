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
  cv::VideoCapture capture = getImageStream(videoPath);
  capture.set(cv::CAP_PROP_FORMAT, CV_32F);

  // cout << capture.get(cv::CAP_PROP_FPS);

  vector<double> queue_density_list, moving_density_list;

  initialize(capture.get(cv::CAP_PROP_FRAME_COUNT));

  parameters.initialize();

  switch (method) {
    case 1:
      method1(queue_density_list, capture, source_pts, frameskip);
      break;
    case 2:
      method2(queue_density_list, capture, source_pts);
      break;
    case 3:
      method3(queue_density_list, capture, source_pts, num_threads);
      break;
    case 4: {
      cv::Mat frame;
      vector<cv::VideoCapture> captures;
      captures.push_back(getImageStream(videoPath));
      captures[0].set(cv::CAP_PROP_FORMAT, CV_32F);
      int num_frames = captures[0].get(cv::CAP_PROP_FRAME_COUNT);

      for (int i = 0; i < num_threads; i++) {
        cv::VideoCapture cap = getImageStream(videoPath);
        int frames_per_thread = ceil((double)(num_frames - 501) / num_threads);
        cap.set(cv::CAP_PROP_FORMAT, CV_32F);
        jump(cap, 501 + i * frames_per_thread);
        // cap.set(cv::CAP_PROP_POS_FRAMES, 501 + i * frames_per_thread);
        captures.push_back(cap);
      }

      captures[0] >> frame;

      parameters.initialize();
      method4(queue_density_list, captures, source_pts, num_threads);
      break;
    }
    case 5:
      method5(moving_density_list, capture, source_pts);
      break;
    case 6:
      method0_md(moving_density_list, capture, source_pts);
      break;
    case 0:
      method0(queue_density_list, capture, source_pts);
      break;
    default:
      // cout << "Go read the documentation" << endl;
      return;
  }

  parameters.complete();

  if (method < 5) {
    double utility_queue = find_utility_qd(queue_density_list, frameskip);
    logger.log("Utility is: " + to_string(utility_queue));
    cout << utility_queue << endl;
  } else {
    double utility_moving = find_utility_md(moving_density_list, frameskip);
    logger.log("Utility is: " + to_string(utility_moving));
    cout << utility_moving << endl;
  }

  make_graph(queue_density_list, moving_density_list, imagePath, frameskip);
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

  parse(argc, argv, imagePath, videoPath, frameskip, choose, method,
        num_threads, logger.enable);

  // cout << logger.enable << endl;

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

  // cout << "time elapsed: " << parameters.get_time_elapsed() << endl;
  cout << parameters.get_time_elapsed() << endl;
  return 0;
}

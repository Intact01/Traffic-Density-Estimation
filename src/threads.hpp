#pragma once
#include <bits/stdc++.h>

#include "graphs.hpp"
#include "image_operations.hpp"
#include "opencv2/opencv.hpp"
#include "properties.hpp"

struct Method4ThreadArgs {
  int rem;
  bagSub pBSub;
  cv::VideoCapture curr_capture;
};

struct Method3ThreadArgs {
  int index;
  bagSub pBSub;
  cv::Rect crop_rect;
  cv::Mat frame;
  double queue_density;
};

vector<bagSub> bgSubs;
std::mutex mtx;

vector<double> qd_list;
vector_point src_pts;
int total_pixels = -1;
int frame_index = 0;
int global_num_threads;

void *threading_frames_method4(void *arguments) {
  cv::Mat frame;
  Method4ThreadArgs *args = (Method4ThreadArgs *)arguments;
  auto [rem, pBSub, capture] = *args;
  int frames_per_thread =
      ceil((double)(capture.get(cv::CAP_PROP_FRAME_COUNT) - 501) /
           global_num_threads);

  int curr_index = 500 + rem * frames_per_thread;
  int temp = capture.get(cv::CAP_PROP_POS_FRAMES);
  while (true) {
    capture >> frame;

    if (curr_index >= 500 + (rem + 1) * frames_per_thread || frame.empty())
      break;

    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, src_pts, dest_pts);
    int queueSum = processQueue(frame, args->pBSub);
    if (total_pixels == -1) {
      total_pixels = frame.rows * frame.cols;
    }
    double queue_density = (double)queueSum / total_pixels;
    qd_list[curr_index] = queue_density;

    stringstream ss;
    ss << curr_index << " " << qd_list[curr_index];
    logger.log(ss.str());

    curr_index++;
  }
  return NULL;
}

void *threading_frames_method3(void *arguments) {
  Method3ThreadArgs *args = (Method3ThreadArgs *)arguments;
  auto [thread_index, pBSub, crop_rect, frame, queue_density] = *args;

  frame = frame(crop_rect);

  int queueSum = processQueue(frame, args->pBSub);
  if (total_pixels == -1) {
    total_pixels = frame.rows * frame.cols;
  }
  args->queue_density = (double)queueSum / total_pixels;

  return NULL;
}

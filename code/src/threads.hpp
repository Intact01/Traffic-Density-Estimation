#pragma once
#include <bits/stdc++.h>

#include "density.hpp"
#include "image_operations.hpp"
#include "opencv2/opencv.hpp"
#include "process.hpp"
#include "properties.hpp"

struct ThreadArgs {
  int frame_index;
  int thread_index;
  cv::Mat frame;
};

class ThreadOperations {
 public:
  int num_threads;
  ThreadArgs method_args;
  bool frame_empty = true;
  bagSub sub;
  vector<double> qd_list;
  vector<bagSub> bag_subs;
  vector<vector<double>> qd_lists;
  vector_point src_pts;

  vector<cv::Rect> rects;
  cv::VideoCapture cap;
  bool completed = false;
  vector<bool> frame_processed;

  static cv::Mat frame;
  static int total_pixels;

  pthread_mutex_t mutex_variable = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t Buffer_Queue_Not_Full = PTHREAD_COND_INITIALIZER;
  pthread_cond_t Buffer_Queue_Not_Empty = PTHREAD_COND_INITIALIZER;

  ThreadOperations(cv::VideoCapture capture, int num_threads);
  void* consumer();
  void* producer_method3();
  void* producer_method4();
  void* threading_frames_method3(void*);
};

struct Method3ThreadArgs {
  int index;
  bagSub pBSub;
  cv::Rect crop_rect;
  cv::Mat frame;
  double queue_density;
};

cv::Mat ThreadOperations::frame;
int ThreadOperations::total_pixels = -1;

// Constructor that initializes all class and object variables
ThreadOperations::ThreadOperations(cv::VideoCapture capture, int num_threads) {
  cap = capture;
  this->num_threads = num_threads;
  cv::Mat dummy_frame;
  cap >> dummy_frame;
  cvtColor(dummy_frame, dummy_frame, cv::COLOR_BGR2GRAY);
  dummy_frame = cameraCorrection(dummy_frame, scr_pts, dest_pts);

  // create lists of rectangles and background subtractors
  for (int i = 0; i < num_threads; i++) {
    cv::Rect rect;

    rect.height = dummy_frame.rows / num_threads;
    rect.width = dummy_frame.cols;
    rect.x = 0;
    rect.y = dummy_frame.rows * i / num_threads;

    rects.push_back(rect);
    bag_subs.push_back(cv::createBackgroundSubtractorMOG2());
  }
  sub = cv::createBackgroundSubtractorMOG2();
  int num_frames = cap.get(cv::CAP_PROP_FRAME_COUNT);
  for (int i = 0; i < num_threads; i++) {
    vector<double> density_list(num_frames - 1, 0);
    qd_lists.push_back(density_list);
  }
  qd_list = vector<double>(num_frames, 0);
  frame_processed = vector<bool>(num_frames, false);
}

void* ThreadOperations::consumer() {
  cv::Mat curr_frame;
  int fidx, tidx;
  while (!completed) {
    pthread_mutex_lock(&mutex_variable);

    // if all frames have been read then release the lock and break
    if (completed) {
      pthread_mutex_unlock(&mutex_variable);
      pthread_cond_signal(&Buffer_Queue_Not_Empty);
      break;
    }
    if (frame_empty) {
      pthread_cond_wait(&Buffer_Queue_Not_Empty, &mutex_variable);
    }

    // read from buffer
    method_args.frame.copyTo(curr_frame);
    fidx = method_args.frame_index;
    tidx = method_args.thread_index;

    frame_processed[fidx] = true;
    frame_empty = true;

    pthread_mutex_unlock(&mutex_variable);
    pthread_cond_signal(&Buffer_Queue_Not_Full);

    if (curr_frame.empty()) {
      pthread_cond_signal(&Buffer_Queue_Not_Empty);
      break;
    }
    int total_pixels;

    // in case of method 4 apply camera correction and in case of method 3 crop
    if (tidx == -1) {
      cvtColor(curr_frame, curr_frame, cv::COLOR_BGR2GRAY);
      curr_frame = cameraCorrection(curr_frame, scr_pts, dest_pts);
      total_pixels = curr_frame.rows * curr_frame.cols;
    } else {
      total_pixels = curr_frame.rows * curr_frame.cols;
      curr_frame = curr_frame(rects[tidx]);
    }

    int queueSum = processQueue(curr_frame, tidx == -1 ? sub : bag_subs[tidx]);
    if (tidx != -1)
      qd_lists[tidx][fidx] = (double)queueSum / total_pixels;
    else
      qd_list[fidx] = (double)queueSum / total_pixels;

    stringstream ss;
    ss << fidx << " " << tidx << " " << (double)queueSum / total_pixels;
    logger.log(ss.str());
  }
  return NULL;
}

void* ThreadOperations::producer_method3() {
  int thread_pos = num_threads, frame_pos = -1;
  cv::Mat curr_frame, modified_frame;

  while (!completed) {
    // after all sections of current image are processed retrieve new image from
    // capture
    if (thread_pos == num_threads) {
      cap >> curr_frame;
      frame_pos++;
      thread_pos = 0;
      if (!curr_frame.empty()) {
        cvtColor(curr_frame, curr_frame, cv::COLOR_BGR2GRAY);
        curr_frame = cameraCorrection(curr_frame, scr_pts, dest_pts);
      }
    }

    pthread_mutex_lock(&mutex_variable);
    if (curr_frame.empty()) {
      // set flag completed to ensure other consumers and current producer do
      // not have further iterations
      completed = true;
    }
    // if buffer is occupied, wait till buffer is read by some consumer
    if (!frame_empty && !completed) {
      pthread_cond_wait(&Buffer_Queue_Not_Full, &mutex_variable);
    }

    if (thread_pos == 0)
      method_args = {frame_pos, thread_pos, curr_frame};
    else
      method_args.thread_index = thread_pos;

    frame_empty = false;

    // remove lock and send signal to consumers to read buffer
    pthread_mutex_unlock(&mutex_variable);
    pthread_cond_signal(&Buffer_Queue_Not_Empty);
    thread_pos++;
  }
  return NULL;
}

// producer method for threading in approach 4 - frame-wise splits
void* ThreadOperations::producer_method4() {
  int frame_pos = 0;

  cv::Mat curr_frame, modified_frame;

  while (!completed) {
    // acquire lock
    pthread_mutex_lock(&mutex_variable);
    while (!frame_empty || (frame_pos > 0 && !frame_processed[frame_pos - 1])) {
      pthread_cond_wait(&Buffer_Queue_Not_Full, &mutex_variable);
    }
    cap >> curr_frame;
    if (curr_frame.empty()) {
      completed = true;
    }

    // write to buffer
    method_args = {frame_pos++, -1, curr_frame};
    frame_empty = false;
    pthread_mutex_unlock(&mutex_variable);
    pthread_cond_signal(&Buffer_Queue_Not_Empty);
  }
  return NULL;
}

// alternate implementation of method 3 threading
void* threading_frames_method3(void* arguments) {
  Method3ThreadArgs* args = (Method3ThreadArgs*)arguments;

  cv::Mat frame = ThreadOperations::frame;
  frame = frame(args->crop_rect);

  int queueSum = processQueue(frame, args->pBSub);
  if (ThreadOperations::total_pixels == -1) {
    ThreadOperations::total_pixels = frame.rows * frame.cols;
  }
  args->queue_density = (double)queueSum / ThreadOperations::total_pixels;

  return NULL;
}

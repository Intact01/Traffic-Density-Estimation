#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"

#include <bits/stdc++.h>

#include "graphs.hpp"
#include "helpers.hpp"
#include "image_operations.hpp"
#include "process.hpp"
#include "properties.hpp"
#include "threads.hpp"

class Density {
 public:
  vector<double> queue_density_list;
  vector<double> moving_density_list;
  cv::VideoCapture capture;
  vector_point source_points;
  int fast_forward = 1;
  int num_threads = 4;
  Resolution resolution;

  void method0_qd();
  void method0_md();
  void method1();
  void method2();
  void method3();
  void method3_previous();
  void method4();
  void method5();
};

void Density::method0_qd() {
  cv::Mat frame;
  capture >> frame;  // capture the first frame

  int counter = 0;
  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  bagSub pBackSub1;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor

  while (true) {
    capture >> frame;  // capture next frame
    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);
    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    stringstream ss;
    ss << left << setw(10) << (counter) << left << setw(10) << queue_density
       << left;
    logger.log(ss.str());
    counter++;
  }
}

void Density::method0_md() {
  cv::Mat frame, prvs, next;
  capture >> frame;  // capture the first frame

  vector<double> original_moving_list;

  int counter = 0;
  int index = 0;

  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  prvs = cameraCorrection(frame, source_points, dest_pts);

  int total_pixels = prvs.rows * prvs.cols;

  double last_k_sum = 0;
  int k = 5;

  while (true) {
    capture >> frame;  // capture next frame

    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);

    // update original motion density list
    int changed_pixels = processMotion(frame, prvs, next);
    double moving_density = (double)changed_pixels / total_pixels;
    original_moving_list.push_back(moving_density);
    index++;

    // take avg with last k frames and update motion density list
    last_k_sum += moving_density;
    if (index > k) last_k_sum -= original_moving_list[index - k];

    double actual_density = last_k_sum / k;
    moving_density_list.push_back(actual_density);

    //  Update the previous frame
    frame.copyTo(prvs);

    stringstream ss;
    ss << left << setw(10) << (counter) << left << setw(10) << actual_density
       << left;
    logger.log(ss.str());
    counter++;
  }
}

void Density::method1() {
  cv::Mat frame;
  capture >> frame;  // capture the first frame

  int counter = 0;
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  bagSub pBackSub1;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor

  while (capture.grab()) {
    if (counter % fast_forward != 0) {
      counter++;
      continue;
    }
    capture.retrieve(frame);

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);
    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    stringstream ss;
    ss << left << setw(10) << (counter) << left << setw(10) << queue_density
       << left;
    logger.log(ss.str());
    counter++;
  }
}

void Density::method2() {
  cv::Mat frame;
  capture >> frame;  // capture the first frame
  auto [width, height] = resolution;

  int counter = 0;
  for (int i = 0; i < 4; i++) {
    cv::Point2f pt;
    pt = dest_pts[i];
    pt.x = pt.x * width / frame.size().width;
    pt.y = pt.y * height / frame.size().height;
    dest_pts[i] = pt;

    pt = source_points[i];
    pt.x = pt.x * width / frame.size().width;
    pt.y = pt.y * height / frame.size().height;
    source_points[i] = pt;
  }
  resize(frame, frame, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);
  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  bagSub pBackSub1;
  pBackSub1 = cv::createBackgroundSubtractorMOG2();  // to create background
                                                     // subtractor
  while (true) {
    capture >> frame;  // capture next frame

    if (frame.empty()) break;
    resize(frame, frame, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);
    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);
    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    stringstream ss;
    ss << counter << " " << queue_density;
    logger.log(ss.str());
    counter++;
  }
}
void Density::method3() {
  if (num_threads == 0) {
    method0_qd();
    return;
  }
  pthread_t consumer_threads[num_threads];
  pthread_t producer_thread;

  capture.set(cv::CAP_PROP_FORMAT, CV_32F);

  ThreadOperations *th_op = new ThreadOperations(capture, num_threads);
  th_op->src_pts = source_points;

  // spawn producer thread
  pthread_create(&producer_thread, NULL,
                 (THREADFUNCPTR)&ThreadOperations::producer_method3, th_op);

  // spawn consumer threads
  for (int i = 0; i < num_threads; i++) {
    int rc = pthread_create(&consumer_threads[i], NULL,
                            (THREADFUNCPTR)&ThreadOperations::consumer, th_op);
    if (rc) {
      cout << "Error:unable to create thread," << rc << endl;
      exit(-1);
    }
  }

  pthread_join(producer_thread, NULL);
  for (int i = 0; i < num_threads; i++) {
    pthread_join(consumer_threads[i], NULL);
  }

  // combine density from all qd_lists
  for (int i = 0; i < th_op->qd_lists[0].size(); i++) {
    double density = 0;
    for (int j = 0; j < num_threads; j++) {
      density += th_op->qd_lists[j][i];
    }
    queue_density_list.push_back(density);
    logger.log(to_string(i) + " " + to_string(density));
  }
}

// older implementation of method 3
void Density::method3_previous() {
  if (num_threads == 0) {
    method0_qd();
    return;
  }
  num_threads++;
  pthread_t threads[num_threads];

  ThreadOperations *th_op = new ThreadOperations(capture, num_threads);

  vector<Method3ThreadArgs> args_array;
  for (int i = 0; i < num_threads; i++) {
    // Method3ThreadArgs args = {i, 0.0, th_op};

    cv::Mat frame;
    Method3ThreadArgs args{i, cv::createBackgroundSubtractorMOG2(),
                           th_op->rects[i], frame, 0.0};
    args_array.push_back(args);
  }

  int counter = 0;

  while (true) {
    cv::Mat frame;
    th_op->cap >> frame;

    if (frame.empty()) break;
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    ThreadOperations::frame = cameraCorrection(frame, source_points, dest_pts);

    for (int i = 0; i < num_threads; i++) {
      int rc = pthread_create(&threads[i], NULL, threading_frames_method3,
                              (void *)(&args_array[i]));
      if (rc) {
        cout << "Error:unable to create thread," << rc << endl;
        exit(-1);
      }
    }

    double total_queue_density = 0;
    for (int i = 0; i < num_threads; i++) {
      pthread_join(threads[i], NULL);
      total_queue_density += args_array[i].queue_density;
    }
    total_queue_density /= num_threads;
    queue_density_list.push_back(total_queue_density);

    stringstream ss;
    ss << counter << " " << total_queue_density;
    logger.log(ss.str());

    counter++;
  }
}

void Density::method4() {
  // if number of threads is 0, redirect to method 0
  if (num_threads == 0) {
    method0_qd();
    return;
  }
  pthread_t consumer_threads[num_threads];
  pthread_t producer_thread;

  capture.set(cv::CAP_PROP_FORMAT, CV_32F);

  ThreadOperations *th_op = new ThreadOperations(capture, num_threads);
  th_op->src_pts = source_points;

  pthread_create(&producer_thread, NULL,
                 (THREADFUNCPTR)&ThreadOperations::producer_method4, th_op);

  for (int i = 0; i < num_threads; i++) {
    int rc = pthread_create(&consumer_threads[i], NULL,
                            (THREADFUNCPTR)&ThreadOperations::consumer, th_op);
    if (rc) {
      cout << "Error:unable to create thread," << rc << endl;
      exit(-1);
    }
  }

  pthread_join(producer_thread, NULL);
  for (int i = 0; i < num_threads; i++) {
    pthread_join(consumer_threads[i], NULL);
  }

  for (auto density : th_op->qd_list) {
    queue_density_list.push_back(density);
  }
}

void Density::method5() {
  cv::Mat prvs, prvs_gray;
  // Take first frame and find corners in it
  capture >> prvs;
  vector<cv::Point2f> p0, p1;
  prvs = cameraCorrection(prvs, source_points, dest_pts);

  int total_pixels = prvs.rows * prvs.cols;
  int counter = 0;
  while (true) {
    cv::Mat frame;
    capture >> frame;
    if (frame.empty()) break;

    frame = cameraCorrection(frame, source_points, dest_pts);
    cv::Mat prvs_copy = prvs.clone(), frame_copy = frame.clone();

    int density_pixels = processMotionSparse(prvs_copy, frame_copy);

    double moving_desnity = (double)density_pixels / total_pixels;
    moving_density_list.push_back(moving_desnity);
    stringstream ss;
    ss << left << setw(10) << counter << left << setw(10) << moving_desnity;
    logger.log(ss.str());

    prvs = frame.clone();

    counter++;
  }
}

#pragma GCC diagnostic pop
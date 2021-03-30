#pragma once

#include <bits/stdc++.h>

#include "graphs.hpp"
#include "helpers.hpp"
#include "image_operations.hpp"
#include "process.hpp"
#include "properties.hpp"
#include "threads.hpp"

// #define NUM_THREADS 5

vector<double> qd_list;
vector_point src_pts;
int total_pixels = -1;
int frame_index = 0;
int global_num_threads;

// void *threading_frames(void *arguments) {
//   cv::Mat frame;
//   Method4ThreadArgs *args = (Method4ThreadArgs *)arguments;
//   auto [rem, pBSub, capture] = *args;
//   int frames_per_thread =
//       ceil((double)(capture.get(cv::CAP_PROP_FRAME_COUNT) - 501) /
//            global_num_threads);

//   int curr_index = 500 + rem * frames_per_thread;
//   int temp = capture.get(cv::CAP_PROP_POS_FRAMES);
//   while (true) {
//     capture >> frame;

//     if (curr_index >= 500 + (rem + 1) * frames_per_thread || frame.empty())
//       break;

//     cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
//     frame = cameraCorrection(frame, src_pts, dest_pts);
//     int queueSum = processQueue(frame, args->pBSub);
//     if (total_pixels == -1) {
//       total_pixels = frame.rows * frame.cols;
//     }
//     double queue_density = (double)queueSum / total_pixels;
//     qd_list[curr_index] = queue_density;

//     stringstream ss;
//     ss << curr_index << " " << qd_list[curr_index];
//     logger.log(ss.str());

//     curr_index++;
//   }
//   return NULL;
// }

void method0(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points) {
  cv::Mat frame;
  capture >> frame;  // capture the first frame

  int counter = 0;
  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  // cout << total_pixels << endl;
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

void method1(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points, int fast_forward = 1) {
  cv::Mat frame;
  capture >> frame;  // capture the first frame

  int counter = 0;
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  // cout << total_pixels << endl;
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

void method0_md(vector<double> &moving_density_list, cv::VideoCapture capture,
                vector_point source_points) {
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

void method2(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points, int width = 480, int height = 360) {
  cv::Mat frame;
  capture >> frame;  // capture the first frame

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
  // cout << total_pixels << endl;
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
    ss << counter << queue_density;
    logger.log(ss.str());
    counter++;
  }
}
void method3(vector<double> &queue_density_list, cv::VideoCapture local_capture,
             vector_point source_points, int num_threads) {
  pthread_t consumer_threads[num_threads];
  pthread_t producer_thread;

  src_pts = source_points;
  local_capture.set(cv::CAP_PROP_FORMAT, CV_32F);

  ThreadOperations *th_op = new ThreadOperations(local_capture, num_threads);

  pthread_create(&producer_thread, NULL,
                 (THREADFUNCPTR)&ThreadOperations::producer_method3, th_op);

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

  for (int i = 0; i < th_op->qd_lists[0].size(); i++) {
    double density = 0;
    for (int j = 0; j < num_threads; j++) {
      density += th_op->qd_lists[j][i];
    }
    queue_density_list.push_back(density);
    logger.log(to_string(i) + " " + to_string(density));
  }
}

void method4(vector<double> &queue_density_list, cv::VideoCapture local_capture,
             vector_point source_points, int num_threads) {
  pthread_t consumer_threads[num_threads];
  pthread_t producer_thread;

  src_pts = source_points;
  local_capture.set(cv::CAP_PROP_FORMAT, CV_32F);

  ThreadOperations *th_op = new ThreadOperations(local_capture, num_threads);

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

  // int i = 0;
  for (auto density : th_op->qd_list) {
    queue_density_list.push_back(density);
    // logger.log(to_string(i++) + " " + to_string(density));
  }
}

void method5(vector<double> &moving_density_list, cv::VideoCapture capture,
             vector_point source_points) {
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

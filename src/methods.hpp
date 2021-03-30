#pragma once

#include <bits/stdc++.h>

#include "density.hpp"
#include "helpers.hpp"
#include "image_operations.hpp"
#include "properties.hpp"
#include "threads.hpp"

void method0(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points) {
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

void method1(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points, int fast_forward = 1) {
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
  pthread_t threads[num_threads];
  int td[num_threads];

  src_pts = source_points;

  cv::Mat test_cap;
  local_capture >> test_cap;
  cvtColor(test_cap, test_cap, cv::COLOR_BGR2GRAY);
  test_cap = cameraCorrection(test_cap, src_pts, dest_pts);

  vector<Method3ThreadArgs> args_array;
  for (int i = 0; i < num_threads; i++) {
    td[i] = i;

    cv::Rect rect;
    rect.height = test_cap.rows / num_threads;
    rect.width = test_cap.cols;
    rect.x = 0;
    rect.y = test_cap.rows * i / num_threads;

    cv::Mat frame;

    Method3ThreadArgs args{td[i], cv::createBackgroundSubtractorMOG2(), rect,
                           frame, 0.0};
    args_array.push_back(args);
  }

  int counter = 0;
  bool completed = false;

  while (true) {
    cv::Mat frame;
    local_capture >> frame;

    if (frame.empty()) break;
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, src_pts, dest_pts);

    for (int i = 0; i < num_threads; i++) {
      args_array[i].frame = frame;
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

void method4(vector<double> &queue_density_list,
             vector<cv::VideoCapture> local_captures,
             vector_point source_points, int num_threads) {
  pthread_t threads[num_threads];
  // pthread_attr_t attr;
  vector<cv::Mat> frames;
  bagSub pBSub;

  global_num_threads = num_threads;

  int td[num_threads];
  pBSub = cv::createBackgroundSubtractorMOG2();

  for (int i = 0; i < -1 + local_captures[0].get(cv::CAP_PROP_FRAME_COUNT);
       i++) {
    qd_list.push_back(0);
  }

  src_pts = source_points;
  cv::Mat frame;
  local_captures[0].set(cv::CAP_PROP_FORMAT, CV_32F);
  int counter = 0;
  while (true) {
    local_captures[0] >> frame;

    if (frame.empty()) {
      break;
    }

    if (counter < 500) {
      cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
      frame = cameraCorrection(frame, src_pts, dest_pts);
      int queueSum = processQueue(frame, pBSub);
      if (total_pixels == -1) {
        total_pixels = frame.rows * frame.cols;
      }
      double queue_density = (double)queueSum / total_pixels;
      qd_list[counter] = queue_density;

      stringstream ss;
      ss << counter << " " << qd_list[counter];
      logger.log(ss.str());
      counter++;

    } else
      break;
  }

  vector<Method4ThreadArgs> args_array;
  for (int i = 0; i < num_threads; i++) {
    td[i] = i;
    Method4ThreadArgs args{td[i], pBSub, local_captures[i + 1]};
    args_array.push_back(args);
  }

  frame_index = counter;
  for (int i = 0; i < num_threads; i++) {
    int rc = pthread_create(&threads[i], NULL, threading_frames_method4,
                            (void *)(&args_array[i]));
    if (rc) {
      cout << "Error:unable to create thread," << rc << endl;
      exit(-1);
    }
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
  queue_density_list = qd_list;
  return;
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
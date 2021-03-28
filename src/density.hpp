#pragma once
#include <pthread.h>

#include <iostream>

#include "graphs.hpp"
#include "image_operations.hpp"
#include "properties.hpp"

// #define NUM_THREADS 5
typedef cv::Ptr<cv::BackgroundSubtractor> bagSub;

vector<bagSub> bgSubs;

struct Method4ThreadArgs {
  int rem;
  bagSub pBSub;
};

struct Method3ThreadArgs {
  int index;
  bagSub pBSub;
  cv::Rect crop_rect;
  cv::Mat frame;
  double queue_density;
};

std::mutex mtx;

vector<double> qd_list;
vector_point src_pts;
int total_pixels = -1;
int frame_index = 0;
vector<cv::VideoCapture> captures;
int global_num_threads;

int processQueue(cv::Mat frame, bagSub pBackSub) {
  cv::Mat frame1, prvs, fgMask;

  // create the structuring element that can be further passed to erode and
  // dilate
  int dilation_size = 2;
  cv::Mat element2 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
      cv::Point(dilation_size, dilation_size));

  mtx.lock();
  pBackSub->apply(frame, fgMask, 0);
  fgMask.copyTo(frame1);
  mtx.unlock();

  // successive erosion and dilation to fill holes in parse
  cv::Mat thresh;
  cv::erode(fgMask, thresh, element2);
  cv::dilate(frame1, frame1, element2);
  cv::erode(fgMask, thresh, element2);
  cv::dilate(frame1, frame1, element2);

  cv::dilate(frame1, frame1, element2);

  // count pixels only greater than 127
  cv::threshold(frame1, frame1, 127, 255, cv::THRESH_BINARY);
  int val = cv::countNonZero(frame1);

  // cv::waitKey(30);
  return val;
}

// calculate motion density with optical flow
int processMotion(cv::Mat frame, cv::Mat prvs, cv::Mat &next) {
  // copy frame to next
  frame.copyTo(next);

  // create the structuring element that can be further passed to erode
  int erosion_size = 2;
  cv::Mat element1 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
      cv::Point(erosion_size, erosion_size));
  // calculate optical flow
  cv::Mat flow(prvs.size(), CV_32FC2);
  cv::calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

  cv::Mat flow_parts[2];
  split(flow, flow_parts);

  // Convert the output into Polar coordinates
  cv::Mat magnitude, angle, magn_norm;
  cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
  cv::normalize(magnitude, magn_norm, 0.0f, 1.0f, cv::NORM_MINMAX);
  angle *= ((1.f / 360.f) * (180.f / 255.f));

  // create image of flow
  cv::Mat _hsv[3], hsv, hsv2, bgr, gray, thresh;
  _hsv[0] = angle;
  _hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
  _hsv[2] = magn_norm;
  cv::merge(_hsv, 3, hsv);
  hsv.convertTo(hsv2, CV_8U, 255.0);

  // convert hsv to gray
  cv::cvtColor(hsv2, bgr, cv::COLOR_HSV2BGR);
  cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

  // keep pixels only greater than 127
  cv::threshold(gray, thresh, 127, 255, cv::THRESH_TRIANGLE);

  // errode and dilate to fill the holes
  cv::erode(thresh, thresh, element1);
  cv::dilate(thresh, thresh, element1);

  // count nonzero pixels
  int changed_pixels = cv::countNonZero(thresh);
  return changed_pixels;
}

// for density calculation
void calc_density(vector<double> &queue_density_list,
                  vector<double> &moving_density_list, cv::VideoCapture capture,
                  int fast_forward, vector_point source_points) {
  cv::Mat frame, prvs, next;
  capture >> frame;  // capture the first frame

  vector<double> original_moving_list;

  int counter = 0;
  int index = 0;

  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  prvs = cameraCorrection(frame, source_points, dest_pts);

  int total_pixels = prvs.rows * prvs.cols;

  bagSub pBackSub1, pBackSub2;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor
  pBackSub2 = cv::createBackgroundSubtractorKNN(
      40);  // to create background subtractor using starting 40 frames

  double last_k_sum = 0;
  int k = 5;

  while (true) {
    counter++;
    capture >> frame;  // capture next frame

    if (counter % fast_forward != 0) continue;
    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);

    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

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

    // show image frame
    imshow("Frame", frame);

    //  Update the previous frame
    frame.copyTo(prvs);
    std::cout << left << setw(10) << double(counter) / 15 << left << setw(10)
              << queue_density << left << setw(10) << moving_density << endl;

    // update graph
    update(queue_density, actual_density);
  }
  std::cout << "Generating final graph... Please Wait" << endl;
}

void *threading_frames(void *arguments) {
  cv::Mat frame;
  Method4ThreadArgs *args = (Method4ThreadArgs *)arguments;
  auto [rem, pBSub] = *args;
  int frames_per_thread = ceil(
      (captures[0].get(cv::CAP_PROP_FRAME_COUNT) - 500) / global_num_threads);

  int curr_index = 500 + rem * frames_per_thread;

  while (true) {
    captures[rem + 1] >> frame;

    if (curr_index >= -1 + 500 + (rem + 1) * frames_per_thread || frame.empty())
      break;

    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, src_pts, dest_pts);
    int queueSum = processQueue(frame, args->pBSub);
    if (total_pixels == -1) {
      total_pixels = frame.rows * frame.cols;
    }
    double queue_density = (double)queueSum / total_pixels;
    qd_list[curr_index] = queue_density;

    // mtx.lock();
    cout << curr_index << " " << qd_list[curr_index] << endl;
    // mtx.unlock();

    curr_index++;
  }
  return NULL;
}

void *threading_frames_method3(void *arguments) {
  Method3ThreadArgs *args = (Method3ThreadArgs *)arguments;
  auto [thread_index, pBSub, crop_rect, frame, queue_density] = *args;

  frame = frame(crop_rect);

  // mtx.lock();
  // imshow("frame " + to_string(thread_index), frame);
  // cv::waitKey(10);
  // mtx.unlock();

  int queueSum = processQueue(frame, args->pBSub);
  if (total_pixels == -1) {
    total_pixels = frame.rows * frame.cols;
  }
  args->queue_density = (double)queueSum / total_pixels;

  return NULL;
}

void method4(vector<double> &queue_density_list,
             vector<cv::VideoCapture> local_captures,
             vector_point source_points, int num_threads) {
  pthread_t threads[num_threads];
  // pthread_attr_t attr;
  vector<cv::Mat> frames;
  bagSub pBSub;

  global_num_threads = num_threads;

  captures = local_captures;

  int td[num_threads];
  pBSub = cv::createBackgroundSubtractorMOG2();
  // qd_list = queue_density_list;
  // qd_list.resize(captures[0].get(cv::CAP_PROP_FRAME_COUNT) - 1);

  for (int i = 0; i < -1 + captures[0].get(cv::CAP_PROP_FRAME_COUNT); i++) {
    qd_list.push_back(0);
  }

  src_pts = source_points;
  cv::Mat frame;
  captures[0].set(cv::CAP_PROP_FORMAT, CV_32F);
  captures[0] >> frame;
  int counter = 0;
  while (true) {
    captures[0] >> frame;

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
      cout << counter << " " << qd_list[counter] << endl;
      counter++;

    } else
      break;
  }

  vector<Method4ThreadArgs> args_array;
  for (int i = 0; i < num_threads; i++) {
    td[i] = i;
    cout << "create thread " << i << endl;
    Method4ThreadArgs args{td[i], pBSub};
    args_array.push_back(args);
  }

  frame_index = counter;
  for (int i = 0; i < num_threads; i++) {
    int rc = pthread_create(&threads[i], NULL, threading_frames,
                            (void *)(&args_array[i]));
    if (rc) {
      cout << "Error:unable to create thread," << rc << endl;
      exit(-1);
    }
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
  cout << "done" << endl;
  queue_density_list = qd_list;
  return;
}
void method0(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points, int fast_forward = 1) {
  cv::Mat frame;
  capture >> frame;  // capture the first frame

  int counter = 0;
  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  cout << total_pixels << endl;
  bagSub pBackSub1;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor

  while (true) {
    counter++;
    capture >> frame;  // capture next frame

    if (counter % fast_forward != 0) continue;
    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);
    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    std::cout << left << setw(10) << (counter) << left << setw(10)
              << queue_density << left << endl;
  }
}

void method2(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points, int width = 176, int height = 144) {
  cv::Mat frame;
  capture >> frame;  // capture the first frame

  int counter = 0;
  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  frame = cameraCorrection(frame, source_points, dest_pts);
  resize(frame, frame, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);
  int total_pixels = frame.rows * frame.cols;
  cout << total_pixels << endl;
  bagSub pBackSub1;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor
  while (true) {
    counter++;
    capture >> frame;  // capture next frame

    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);
    resize(frame, frame, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);
    imshow("method2", frame);
    cv::waitKey(10);
    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    std::cout << (counter) << queue_density << endl;
  }
}

void method1(vector<double> &queue_density_list, cv::VideoCapture capture,
             vector_point source_points) {}

void method3(vector<double> &queue_density_list, cv::VideoCapture local_capture,
             vector_point source_points, int num_threads) {
  pthread_t threads[num_threads];
  int td[num_threads];

  src_pts = source_points;
  // local_capture.set(cv::CAP_PROP_FORMAT, CV_32F);

  cv::Mat test_cap;
  local_capture >> test_cap;
  cvtColor(test_cap, test_cap, cv::COLOR_BGR2GRAY);
  test_cap = cameraCorrection(test_cap, src_pts, dest_pts);
  cout << "test_cap size " << test_cap.size() << endl;

  vector<Method3ThreadArgs> args_array;
  for (int i = 0; i < num_threads; i++) {
    td[i] = i;
    cout << "create thread " << i << endl;

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

  // for(int i = )

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
    cout << counter << " " << total_queue_density << endl;
    counter++;
  }
}

void method5(vector<double> &moving_density_list, cv::VideoCapture capture,
             vector_point source_points) {}
#pragma once
#include <iostream>

#include "graphs.hpp"
#include "image_operations.hpp"
#include "properties.hpp"

typedef cv::Ptr<cv::BackgroundSubtractor> bagSub;

// calculate queue density
int processQueue(cv::Mat frame, bagSub pBackSub) {
  cv::Mat frame1, prvs, fgMask;

  // create the structuring element that can be further passed to erode and
  // dilate
  int dilation_size = 2;
  cv::Mat element2 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
      cv::Point(dilation_size, dilation_size));
  pBackSub->apply(frame, fgMask, 0);
  fgMask.copyTo(frame1);

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

  cv::waitKey(30);
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
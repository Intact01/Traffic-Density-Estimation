#pragma once

#include <bits/stdc++.h>

#include "graphs.hpp"
#include "helpers.hpp"
#include "image_operations.hpp"

std::mutex mtx;

int processQueue(cv::Mat frame, bagSub pBackSub) {
  cv::Mat frame1, prvs, fgMask;

  // create the structuring element that can be further passed to erode and
  // dilate
  int dilation_size = 2;
  cv::Mat element2 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
      cv::Point(dilation_size, dilation_size));

  // mtx.lock();
  pBackSub->apply(frame, fgMask, 0);
  fgMask.copyTo(frame1);
  // mtx.unlock();

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

int processMotionSparse(cv::Mat prvs, cv::Mat frame) {
  int dilation_size = 3;
  cv::Mat element2 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
      cv::Point(dilation_size, dilation_size));

  vector<cv::Point2f> p0, p1;
  cv::Mat prvs_gray, frame_gray;
  cv::cvtColor(prvs, prvs_gray, cv::COLOR_BGR2GRAY);
  cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
  cv::goodFeaturesToTrack(frame_gray, p0, 300, 0.1, 7, cv::Mat(), 7, false,
                          0.04);
  cv::Mat mask = cv::Mat::zeros(frame.size(), frame.type());
  imshow("blank", mask);
  vector<uchar> status;
  vector<float> err;
  cv::TermCriteria criteria = cv::TermCriteria(
      (cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 10, 0.03);
  cv::calcOpticalFlowPyrLK(prvs_gray, frame_gray, p0, p1, status, err,
                           cv::Size(20, 20), 2, criteria);

  for (uint i = 0; i < p0.size(); i++) {
    if (status[i] == 1) {
      if (dist(p1[i], p0[i]) >= 0.5) {
        line(mask, p1[i], p0[i], WHITE, 5);
      }

      circle(frame, p1[i], 5, BLACK, -1);
    }
  }
  // imshow("undilated", mask);
  cv::dilate(mask, mask, element2);
  cv::dilate(mask, mask, element2);
  cv::dilate(mask, mask, element2);
  cv::dilate(mask, mask, element2);
  cv::dilate(mask, mask, element2);
  cv::dilate(mask, mask, element2);

  cvtColor(mask, mask, cv::COLOR_BGR2GRAY);
  // imshow("prvs", prvs);
  // imshow("frame", frame);
  // imshow("mask", mask);
  // cv::waitKey(500);
  return cv::countNonZero(mask);
}
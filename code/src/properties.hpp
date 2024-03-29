#pragma once

#include <iostream>

#include "opencv2/opencv.hpp"

using namespace std;

// This contains all type definitions and constant that are required throughout
// the project

typedef std::vector<cv::Point2f> vector_point;
typedef cv::Point2f Pt;
typedef cv::Ptr<cv::BackgroundSubtractor> bagSub;
typedef void *(*THREADFUNCPTR)(void *);

cv::Scalar WHITE(255, 255, 255);
cv::Scalar BLACK(0, 0, 0);
cv::Scalar ORANGE(0, 165, 255);
cv::Scalar CYAN(255, 165, 0);
vector<cv::Scalar> Colors;

vector_point scr_pts{Pt(1267, 217), Pt(980, 224), Pt(498, 985), Pt(1514, 969)};
vector_point dest_pts{Pt(472, 52), Pt(472, 830), Pt(800, 52), Pt(800, 830)};

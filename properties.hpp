#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;

typedef std::vector<cv::Point2f> vector_point;
typedef cv::Point2f Pt;

vector_point scr_pts{Pt(1292, 210), Pt(1546, 939), Pt(101, 880), Pt(777, 224)};
vector_point dest_pts{Pt(472, 52), Pt(472, 830), Pt(800, 52), Pt(800, 830)};
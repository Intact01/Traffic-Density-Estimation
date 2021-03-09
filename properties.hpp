#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;

typedef std::vector<cv::Point2f> vector_point;
typedef cv::Point2f Pt;

vector_point scr_pts{Pt(1267, 217), Pt(980, 224), Pt(498, 985), Pt(1514, 969)};
vector_point dest_pts{Pt(472, 52), Pt(472, 830), Pt(800, 52), Pt(800, 830)};

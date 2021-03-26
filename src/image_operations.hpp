#pragma once
#include "opencv2/opencv.hpp"

using namespace std;

typedef vector<cv::Point2f> vector_point;
typedef cv::Point2f Pt;

// comparator functions used in sorting
int cmp_x(const cv::Point2f &lhs, const cv::Point2f &rhs) {
  return lhs.x < rhs.x;
}
int cmp_y(const cv::Point2f &lhs, const cv::Point2f &rhs) {
  return lhs.y < rhs.y;
}

// returns the rectangle described by given points
cv::Rect getRectFromPoints(vector_point points) {
  cv::Rect rect;
  rect.width = points[3].x - points[0].x;
  rect.height = points[1].y - points[0].y;
  rect.x = points[0].x;
  rect.y = points[0].y;

  return rect;
}

// Finds the smallest rectangle that contains the image
cv::Rect getBoundingRectangle(cv::Mat img) {
  cv::Mat gray, thresh;
  vector<vector<cv::Point>> contours;
  img.copyTo(gray);
  cv::threshold(gray, thresh, 20, 255, cv::THRESH_BINARY);
  cv::findContours(thresh, contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);
  if (contours.size() > 1) {
    int h = img.size().height;
    int w = img.size().width;
    cv::Rect rect = {0, 0, w, h};
    return rect;
  }
  cv::Rect rect = cv::boundingRect(contours[0]);
  return rect;
}

// Tranformes perspective of original image
// Stores transformed image
cv::Mat cameraCorrection(cv::Mat original_image, vector_point source_pts,
                         vector_point destination_pts) {
  stable_sort(source_pts.begin(), source_pts.end(), cmp_x);
  stable_sort(source_pts.begin(), source_pts.begin() + 2, cmp_y);
  stable_sort(source_pts.begin() + 2, source_pts.begin() + 4, cmp_y);

  // sort destination_pts
  stable_sort(destination_pts.begin(), destination_pts.end(), cmp_x);
  stable_sort(destination_pts.begin(), destination_pts.begin() + 2, cmp_y);
  stable_sort(destination_pts.begin() + 2, destination_pts.begin() + 4, cmp_y);

  // calculate perspective transformation matrix
  cv::Mat homography_matrix = cv::findHomography(source_pts, destination_pts);

  cv::Mat output_image;  // Transformed image
  // Transform perpective of original_image to output_image and store
  // output_image to device
  try {
    cv::warpPerspective(original_image, output_image, homography_matrix,
                        original_image.size());
  } catch (cv::Exception e) {
    std::cout << e.what() << endl;
    return output_image;
  }
  cv::Rect cropRect = getRectFromPoints(destination_pts);
  cv::Mat cropped_image = output_image(cropRect);
  return cropped_image;
}

double dist(cv::Point2f p1, cv::Point2f p2){
  return pow(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2),0.5);
}
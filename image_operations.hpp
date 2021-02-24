#pragma once
#include "opencv2/opencv.hpp"

using namespace std;

// crops given image img into a rectanglular area described by given points
// stores cropped image
cv::Mat crop_image(cv::Mat img, vector<cv::Point2f> points)
{
    // crop area
    cv::Rect crop_area;
    crop_area.width = points[3].x - points[0].x;
    crop_area.height = points[1].y - points[0].y;
    crop_area.x = points[0].x;
    crop_area.y = points[0].y;

    // crop the image
    // show and store the cropped image
    cv::Mat cropped_image = img(crop_area);
    return cropped_image;
}
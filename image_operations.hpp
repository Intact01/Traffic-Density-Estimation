#pragma once
#include "opencv2/opencv.hpp"

using namespace std;

// returns the rectangle described by given points
cv::Rect getRectFromPoints(vector<cv::Point2f> points)
{
    cv::Rect rect;
    rect.width = points[3].x - points[0].x;
    rect.height = points[1].y - points[0].y;
    rect.x = points[0].x;
    rect.y = points[0].y;

    return rect;
}

// Finds the smallest rectangle that contains the image
cv::Rect getBoundingRectangle(cv::Mat img){
    cv::Mat gray,thresh;
    vector<vector<cv::Point>> contours;
    img.copyTo(gray);
    cv::threshold(gray,thresh,20,255,cv::THRESH_BINARY);
    cv::findContours(thresh,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    if (contours.size() > 1){
        int h = img.size().height;
        int w = img.size().width;
        cv::Rect rect = {0,0,w,h};
        return rect;
    }
    cv::Rect rect = cv::boundingRect(contours[0]);
    return rect;
}
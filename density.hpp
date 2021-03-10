#pragma once
#include <iostream>
#include "properties.hpp"
#include "graphs.hpp"

typedef cv::Ptr<cv::BackgroundSubtractor> bagSub;

int processQueue(cv::Mat frame, bagSub pBackSub)
{
    cv::Mat frame1, prvs, fgMask;

    int erosion_size = 1;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    int dilation_size = 2;
    cv::Mat element2 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                                                 cv::Point(dilation_size, dilation_size));
    pBackSub->apply(frame, fgMask, 0);
    fgMask.copyTo(frame1);
    cv::Mat thresh;
    cv::erode(fgMask, thresh, element2);
    cv::dilate(frame1, frame1, element2);
    cv::erode(fgMask, thresh, element2);
    cv::dilate(frame1, frame1, element2);

    cv::dilate(frame1, frame1, element2);

    cv::threshold(frame1, frame1, 127, 255, cv::THRESH_BINARY);

    int val = cv::countNonZero(frame1);

    cv::waitKey(30);
    return val;
}

int processMotion(cv::Mat frame, cv::Mat prvs, cv::Mat &next)
{
    // cv::cvtColor(frame, next, cv::COLOR_BGR2GRAY);

    frame.copyTo(next);

    int erosion_size = 2;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    cv::Mat flow(prvs.size(), CV_32FC2);
    cv::calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

    // Visualization part
    cv::Mat flow_parts[2];
    split(flow, flow_parts);

    // Convert the algorithm's output into Polar coordinates
    cv::Mat magnitude, angle, magn_norm;

    cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
    cv::normalize(magnitude, magn_norm, 0.0f, 1.0f, cv::NORM_MINMAX);
    angle *= ((1.f / 360.f) * (180.f / 255.f));

    cv::Mat _hsv[3], hsv, hsv8, bgr, gray, thresh;

    _hsv[0] = angle;
    _hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
    _hsv[2] = magn_norm;

    cv::merge(_hsv, 3, hsv);
    hsv.convertTo(hsv8, CV_8U, 255.0);

    cv::cvtColor(hsv8, bgr, cv::COLOR_HSV2BGR);
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;

    cv::threshold(gray, thresh, 127, 255, cv::THRESH_TRIANGLE);

    cv::erode(thresh, thresh, element1);
    cv::dilate(thresh, thresh, element1);

    cv::waitKey(30);
    int changed_pixels = cv::countNonZero(thresh);

    return changed_pixels;
}

int processMotion2(cv::Mat frame, cv::Mat prvs, cv::Mat &next)
{
    int erosion_size = 1;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    frame.copyTo(next);

    cv::Mat diff, thresh;
    cv::absdiff(frame, prvs, diff);

    cv::threshold(diff, thresh, 25, 255, cv::THRESH_BINARY);
    cv::dilate(thresh, thresh, element1);

    cv::waitKey(30);
    int changed_pixels = cv::countNonZero(thresh);

    return changed_pixels;
}

int processMotion3(bagSub pBackSub, cv::Mat frame)
{
    cv::Mat frame1, prvs, fgMask;

    int erosion_size = 2;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    pBackSub->apply(frame, fgMask, 0.9);

    fgMask.copyTo(frame1);
    cv::Mat thresh;
    cv::threshold(frame1, frame1, 25, 255, cv::THRESH_BINARY);

    cv::dilate(frame1, frame1, element1);
    int val = cv::countNonZero(frame1);

    cv::waitKey(30);
    return val;
}
void averageMovingDensity(vector<double> &moving_density_list)
{
    for (int i = 2; i < moving_density_list.size() - 2; ++i)
    {
        moving_density_list[i] = (moving_density_list[i - 2] + moving_density_list[i - 1] + moving_density_list[i] + moving_density_list[i + 1] + moving_density_list[i + 2]) / 5;
    }
}

void calc_density(vector<double> &queue_density_list, vector<double> &moving_density_list, cv::VideoCapture capture, int fast_forward, vector_point source_points)
{
    cv::Mat frame, prvs, next;
    capture >> frame;

    vector<double> original_moving_list;

    int counter = 0;
    int index = 0;

    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    prvs = cameraCorrection(frame, source_points, dest_pts);

    int total_pixels = prvs.rows * prvs.cols;

    bagSub pBackSub1, pBackSub2;
    pBackSub1 = cv::createBackgroundSubtractorMOG2();
    pBackSub2 = cv::createBackgroundSubtractorKNN(40);

    double last_k_sum = 0;
    int k = 5;

    while (true)
    {
        counter++;
        capture >> frame;
        if (counter % fast_forward != 0)
            continue;
        if (frame.empty())
            break;

        cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
        frame = cameraCorrection(frame, source_points, dest_pts);

        int queueSum = processQueue(frame, pBackSub1);
        double queue_density = (double)queueSum / total_pixels;
        queue_density_list.push_back(queue_density);

        int changed_pixels = processMotion(frame, prvs, next);

        double moving_density = (double)changed_pixels / total_pixels;
        original_moving_list.push_back(moving_density);
        index++;

        last_k_sum += moving_density;
        if (index > k)
            last_k_sum -= original_moving_list[index - k];

        double actual_density = last_k_sum / k;
        moving_density_list.push_back(actual_density);

        imshow("Frame", frame);

        //  Update the previous frame
        frame.copyTo(prvs);
        std::cout << left << setw(10) << double(counter) / 15 << left << setw(10) << queue_density << left << setw(10) << moving_density << endl;

        update(queue_density, actual_density);
    }
    std::cout << "Generating final graph... Please Wait" << endl;
}
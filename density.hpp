#pragma once
#include <iostream>
#include "properties.hpp"

typedef cv::Ptr<cv::BackgroundSubtractor> bagSub;

int processQueue(cv::Mat frame, bagSub pBackSub)
{
    cv::Mat frame1, prvs, fgMask;

    int erosion_size = 1;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    int dilation_size = 3;
    cv::Mat element2 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                                                 cv::Point(dilation_size, dilation_size));
    pBackSub->apply(frame, fgMask, 0);
    fgMask.copyTo(frame1);
    cv::Mat thresh;
    // cv::erode(fgMask, thresh, element1);
    // cv::threshold(thresh, thresh, 207, 255, cv::THRESH_BINARY);
    //
    // cv::dilate(thresh, frame1, element2);
    // cv::normalize(fgMask, frame1, 0.0f, 1.0f, cv::NORM_MINMAX);
    // thresh.copyTo(frame1);
    double sum = cv::sum(frame1)[0];
    // cout << sum << " " << counter << endl;
    // int val = cv::countNonZero(frame1);
    // cout << val << " " << counter << endl;

    // cv::imshow("Frame", frame);
    // cv::imshow("FG Mask", fgMask);
    // cv::imshow("frame1", frame1);

    // int keyboard = cv::waitKey(30);
    // if (keyboard == 'q' || keyboard == 27)
    return sum;
}

int processMotion(cv::Mat frame, cv::Mat prvs, cv::Mat &next)
{
    // cv::cvtColor(frame, next, cv::COLOR_BGR2GRAY);

    frame.copyTo(next);

    int erosion_size = 5;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    // cv::imshow("prvs", prvs);
    // cv::imshow("next", next);
    // cv::waitKey(0);
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

    cv::Mat _hsv[3], hsv, hsv8, bgr, grey, thresh;

    _hsv[0] = angle;
    _hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
    _hsv[2] = magn_norm;

    cv::merge(_hsv, 3, hsv);
    hsv.convertTo(hsv8, CV_8U, 255.0);
    cv::cvtColor(hsv8, bgr, cv::COLOR_HSV2BGR);
    cv::cvtColor(bgr, grey, cv::COLOR_BGR2GRAY);
    cv::dilate(grey, grey, element1);
    cv::threshold(grey, thresh, 40, 255, cv::THRESH_BINARY);

    cv::imshow("flow", bgr);
    cv::imshow("grey", grey);
    cv::imshow("frame", frame);
    cv::imshow("thresh", thresh);

    cv::waitKey(30);
    int changed_pixels = cv::countNonZero(thresh);

    return changed_pixels;
}
void averageMovingDensity(vector<double> &moving_density_list)
{
    for (int i = 2; i < moving_density_list.size() - 2; ++i)
    {
        moving_density_list[i] = (moving_density_list[i - 2] + moving_density_list[i - 1] + moving_density_list[i] + moving_density_list[i + 1] + moving_density_list[i + 2]) / 5;
    }
}

void calc_density(vector<double> &queue_density_list, vector<double> &moving_density_list, cv::VideoCapture capture, int fast_forward)
{
    cv::Mat frame, prvs, next;
    capture >> frame;

    int counter = 0;

    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    prvs = cameraCorrection(frame, scr_pts, dest_pts);

    // vector<double> queue_density_list;
    // vector<double> moving_density_list;

    int total_pixels = prvs.rows * prvs.cols;

    bagSub pBackSub;
    pBackSub = cv::createBackgroundSubtractorMOG2(1500, false);

    while (true)
    {
        counter++;
        capture >> frame;
        if (counter % fast_forward != 0)
            continue;
        if (frame.empty())
            break;

        cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
        frame = cameraCorrection(frame, scr_pts, dest_pts);

        int queueSum = processQueue(frame, pBackSub);
        double queueDensity = (double)queueSum / 255.0 / total_pixels;
        queue_density_list.push_back(queueDensity);

        int changed_pixels = processMotion(frame, prvs, next);
        double moving_density = (double)changed_pixels / total_pixels;
        moving_density_list.push_back(moving_density);

        //  Update the previous frame
        next.copyTo(prvs);
        cout << queueDensity << " " << changed_pixels << " " << counter << endl;
    }
    cout << "Voila" << endl;
    averageMovingDensity(moving_density_list);
    // cout << queue_density_list;
    // return queue_density_list;
}
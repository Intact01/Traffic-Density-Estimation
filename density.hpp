#pragma once
#include <iostream>
#include "properties.hpp"

vector<double> queue_density(cv::VideoCapture capture, int fast_forward)
{
    cv::Mat frame, init, frame1, prvs, fgMask;
    capture >> frame1;

    int counter = 0;

    cvtColor(frame1, init, cv::COLOR_BGR2GRAY);
    frame1 = cameraCorrection(frame1, scr_pts, dest_pts);

    vector<double> queue_density_list;
    int total_pixels = frame1.rows * frame1.cols;

    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
    pBackSub = cv::createBackgroundSubtractorMOG2(1500, false);

    while (true)
    {
        capture >> frame;

        if (counter++ % fast_forward != 0)
            continue;

        if (frame.empty())
            break;

        cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
        frame = cameraCorrection(frame, scr_pts, dest_pts);

        int erosion_size = 1;
        cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                     cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                     cv::Point(erosion_size, erosion_size));
        int dilation_size = 3;
        cv::Mat element2 = cv::getStructuringElement(cv::MORPH_RECT,
                                                     cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                                                     cv::Point(dilation_size, dilation_size));
        pBackSub->apply(frame, fgMask, 0);

        // fgMask.copyTo(frame1);
        cv::Mat thresh;
        cv::erode(fgMask, thresh, element1);
        cv::threshold(thresh, thresh, 207, 255, cv::THRESH_BINARY);
        //
        cv::dilate(thresh, frame1, element2);
        // cv::normalize(fgMask, frame1, 0.0f, 1.0f, cv::NORM_MINMAX);
        // thresh.copyTo(frame1);
        // double sum = cv::sum(frame1)[0];
        // cout << sum << " " << counter << endl;
        int val = cv::countNonZero(frame1);
        queue_density_list.push_back(val);
        // cout << val << " " << counter << endl;

        imshow("Frame", frame);
        imshow("FG Mask", fgMask);
        // imshow("thresh", thresh);
        imshow("frame1", frame1);

        int keyboard = cv::waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }
    cout << "Voila" << endl;
    // cout << queue_density_list;
    return queue_density_list;
}
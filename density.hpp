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
    //
    // cv::dilate(frame1, frame1, element2);
    // cv::normalize(fgMask, frame1, 0.0f, 1.0f, cv::NORM_MINMAX);
    // thresh.copyTo(frame1);
    // double sum = cv::sum(frame1)[0];
    // cout << sum << " " << counter << endl;
    int val = cv::countNonZero(frame1);
    // cout << val << " " << counter << endl;

    // cv::imshow("Frame", frame);
    // cv::imshow("FG Mask", fgMask);
    // cv::imshow("frame1", frame1);

    int keyboard = cv::waitKey(30);
    // if (keyboard == 'q' || keyboard == 27)
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

    // cv::medianBlur(hsv8, hsv8, 3);

    // cv::dilate(hsv8, hsv8, element1);
    // cv::erode(hsv8, hsv8, element1);
    // cv::dilate(hsv8, hsv8, element1);
    // cv::erode(hsv8, hsv8, element1);

    // cv::dilate(hsv8, hsv8, element1);

    cv::cvtColor(hsv8, bgr, cv::COLOR_HSV2BGR);
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

    // cv::adaptiveThreshold(gray, thresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 21, 5);

    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;

    cv::threshold(gray, thresh, 127, 255, cv::THRESH_TRIANGLE);

    // cv::imshow("thresh0", thresh);

    cv::erode(thresh, thresh, element1);
    cv::dilate(thresh, thresh, element1);

    // cv::findContours(thresh, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    // int contour_area = 0;
    // // cout << "afa" << contours.size() << endl;
    // for (vector<cv::Point> contour : contours)
    // {
    //     contour_area += cv::contourArea(contour);
    //     // cout << "cont" << contour_area << endl;
    // }

    // cv::imshow("flow", bgr);
    // cv::imshow("gray", gray);
    // cv::imshow("frame", frame);
    // cv::imshow("thresh", thresh);

    cv::waitKey(30);
    int changed_pixels = cv::countNonZero(thresh);

    return changed_pixels;
}

int processMotion2(cv::Mat frame, cv::Mat prvs, cv::Mat &next)
{
    // cv::cvtColor(frame, next, cv::COLOR_BGR2GRAY);
    int erosion_size = 1;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    frame.copyTo(next);

    cv::Mat diff, thresh;
    cv::absdiff(frame, prvs, diff);

    // vector<vector<cv::Point>> contours;
    // vector<cv::Vec4i> hierarchy;

    cv::threshold(diff, thresh, 25, 255, cv::THRESH_BINARY);
    cv::dilate(thresh, thresh, element1);

    // cv::findContours(thresh, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    // int contour_area = 0;
    // // cout << "afa" << contours.size() << endl;
    // for (vector<cv::Point> contour : contours)
    // {
    //     contour_area += cv::contourArea(contour);
    //     // cout << "cont" << contour_area << endl;
    // }

    // cv::imshow("flow", bgr);
    // cv::imshow("gray", gray);
    // cv::imshow("frame", frame);
    // cv::imshow("diff", diff);
    // cv::imshow("thresh", thresh);

    cv::waitKey(30);
    int changed_pixels = cv::countNonZero(thresh);

    return changed_pixels;
}

int processMotion3(bagSub pBackSub, cv::Mat frame)
{
    // cv::cvtColor(frame, next, cv::COLOR_BGR2GRAY);
    cv::Mat frame1, prvs, fgMask;

    int erosion_size = 2;
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT,
                                                 cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                 cv::Point(erosion_size, erosion_size));
    pBackSub->apply(frame, fgMask, 0.9);

    fgMask.copyTo(frame1);
    cv::Mat thresh;
    // cv::erode(fgMask, thresh, element1);
    cv::threshold(frame1, frame1, 25, 255, cv::THRESH_BINARY);

    //
    cv::dilate(frame1, frame1, element1);
    // cv::normalize(fgMask, frame1, 0.0f, 1.0f, cv::NORM_MINMAX);
    // thresh.copyTo(frame1);
    // double sum = cv::sum(frame1)[0];
    // cout << sum << " " << counter << endl;
    int val = cv::countNonZero(frame1);
    // cout << val << " " << counter << endl;

    // cv::imshow("Frame", frame);
    // cv::imshow("FG Mask", fgMask);
    // cv::imshow("frame2", frame1);
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

    // vector<double> queue_density_list;
    // vector<double> moving_density_list;

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
        // double queue_density = (double)queueSum / 255.0 / total_pixels;
        double queue_density = (double)queueSum / total_pixels;
        queue_density_list.push_back(queue_density);

        int changed_pixels = processMotion(frame, prvs, next);
        // int changed_pixels = processMotion3(pBackSub2, frame);

        double moving_density = (double)changed_pixels / total_pixels;
        original_moving_list.push_back(moving_density);
        index++;

        last_k_sum += moving_density;
        if (index > k)
            last_k_sum -= original_moving_list[index - k];

        // if (counter < 30)
        //     moving_density = 0;
        double actual_density = last_k_sum / k;
        moving_density_list.push_back(actual_density);

        imshow("Frame", frame);

        //  Update the previous frame
        frame.copyTo(prvs);
        std::cout << left << setw(10) << double(counter) / 15 << left << setw(10) << queue_density << left << setw(10) << moving_density << endl;

        update(queue_density, actual_density);
    }
    std::cout << "Generating final graph... Please Wait" << endl;
    // averageMovingDensity(moving_density_list);
    // cout << queue_density_list;
    // return queue_density_list;
}
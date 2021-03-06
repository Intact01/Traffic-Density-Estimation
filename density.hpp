#pragma once
#include <iostream>
#include "matplotlibcpp.h"

using namespace std;
namespace plt = matplotlibcpp;

typedef vector<cv::Point2f> vector_point;
vector_point scr_pts{Pt(1292, 210), Pt(1546, 939), Pt(101, 880), Pt(777, 224)};
vector_point dest_pts{Pt(472, 52), Pt(472, 830), Pt(800, 52), Pt(800, 830)};

void make_graph(vector<double> moving_density_list)
{
    int n = 5000;
    vector<double> time;

    // std::vector<double> x(n), y(n), z(n), w(n, 2);

    for (int i = 0; i < moving_density_list.size(); ++i)
    {
        // x.at(i) = i*i;
        // y.at(i) = sin(2*M_PI*i/360.0);
        // z.at(i) = log(i);
        time.push_back(i);
    }

    // Set the size of output image to 1200x780 pixels
    plt::figure_size(1200, 780);
    // Plot line from given x and y data. Color is selected automatically.
    plt::named_plot("Moving Density", time, moving_density_list);
    // Plot a red dashed line from given x and y data.
    // plt::plot(x, w,"r--");
    // // Plot a line whose name will show up as "log(x)" in the legend.
    // plt::named_plot("log(x)", x, z);
    // Set x-axis to interval [0,1000000]
    plt::xlim(0, (int)moving_density_list.size());
    // Add graph title
    plt::title("Traffic Density Plot");
    // Enable legend.
    plt::legend();
    // Save the image (file format is determined by the extension)
    plt::save("./basic.png");
}

void queue_density(cv::VideoCapture capture, cv::Mat background, int fast_forward)
{
    cv::Mat frame, init, frame1, prvs, fgMask;
    capture >> frame1;

    int counter = 0;

    cvtColor(frame1, init, cv::COLOR_BGR2GRAY);
    frame1 = cameraCorrection(frame1, scr_pts, dest_pts);

    background = cameraCorrection(background, scr_pts, dest_pts);

    // cv::imshow("b", background);
    // cv::waitKey(0);

    vector<double> queue_density_list;
    int total_pixels = frame1.rows * frame1.cols;

    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
    pBackSub = cv::createBackgroundSubtractorMOG2(false);

    while (true)
    {
        capture >> frame;

        if (counter++ % fast_forward != 0)
            continue;

        if (frame.empty())
            break;

        cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
        frame = cameraCorrection(frame, scr_pts, dest_pts);

        pBackSub->apply(frame, fgMask, 0);

        // frame1 = frame - background;
        // cv::threshold(frame1, frame1, 30, 255, cv::THRESH_BINARY);

        //get the input from the keyboard
        // int changed_pixels = 0;
        // int rows = frame1.rows;
        // int columns = frame1.cols;
        // for (int i = 0; i < rows; i++)
        // {
        //     for (int j = 0; j < columns; j++)
        //     {
        //         if (frame1.at<double>(i, j) > 10)
        //         {
        //             changed_pixels++;
        //         }
        //     }
        // }
        // int val = (0.0 + changed_pixels) / total_pixels;
        fgMask.copyTo(frame1);
        // cv::threshold(fgMask, frame1, 127, 255, cv::THRESH_BINARY);
        // cv::normalize(fgMask, frame1, 0.0f, 1.0f, cv::NORM_MINMAX);
        double sum = cv::sum(frame1)[0];
        cout << sum << " " << counter << endl;
        // int val = cv::countNonZero(frame1);
        queue_density_list.push_back(sum);
        // cout << val << " " << counter << endl;

        imshow("Frame", frame);
        imshow("FG Mask", fgMask);
        imshow("thresh", frame1);

        int keyboard = cv::waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }
    cout << "Voila" << endl;
    // cout << queue_density_list;
    make_graph(queue_density_list);
}
#pragma once
#include "opencv2/opencv.hpp"
using namespace std;

int points_selected = 0;
vector_point source_points;
void (*processImage)(vector_point);

cv::Mat source_image, cache_image;
cv::Scalar green = cv::Scalar(0, 255, 0);

void mouse_callback(int event, int x, int y, int flag, void *param);

void getCustomPoints(void (*main_callback)(vector_point))
{
    source_image = cv::imread("empty.jpg", cv::IMREAD_GRAYSCALE);
    cache_image = source_image.clone();
    cv::imshow("selection", source_image);
    cv::setMouseCallback("selection", mouse_callback);
    processImage = main_callback;
    cv::waitKey(0);
}

void mouse_callback(int event, int x, int y, int flag, void *param)
{
    cv::Point2f newpt = cv::Point2f(x, y);

    // left click
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        points_selected++;

        //till 4 points are selected
        if (points_selected <= 4)
        {
            source_points.push_back(newpt);

            //display circle at clicked point
            cv::circle(source_image, newpt, 5, green, 2, 8, 0);
            cv::imshow("selection", source_image);
        }
        if (points_selected > 1 && points_selected <= 4)
        {
            // join clicked point with previously clicked point
            cv::line(source_image, source_points[points_selected - 2], source_points[points_selected - 1], green, 2, 8, 0);
            cv::imshow("selection", source_image);
        }

        cout << x << " " << y << endl;
        if (points_selected == 4)
        {
            std::cout << "==============" << '\n';
            cv::destroyWindow("selection");
            processImage(source_points);
            cv::destroyAllWindows();
        }
    }

    // mouse pointer tracking
    else if (event == cv::EVENT_MOUSEMOVE)
    {
        //until 4 points are selected
        if (points_selected > 0 && points_selected < 4)
        {
            // show mouse pointer by a line joined to previously clicked point
            source_image.copyTo(cache_image);
            cv::line(cache_image, source_points[points_selected - 1], newpt, green, 2, 8, 0);
            cv::imshow("selection", cache_image);
        }
    }
}
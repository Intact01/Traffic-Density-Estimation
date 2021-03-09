#include <iostream>
#include <cmath>

#include "arg_parser.hpp"
#include "image_operations.hpp"
#include "density.hpp"
#include "choice.hpp"

int frameRate;
string videoPath;
string imagePath;

cv::VideoCapture getImageStream(string videoPath)
{
    cv::VideoCapture capture(videoPath);

    if (!capture.isOpened())
    {
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
    }
    return capture;
}

void start(vector_point source_pts = scr_pts)
{
    cout << "in start" << endl;
    cv::VideoCapture capture = getImageStream(videoPath);
    vector<double> queue_density_list, moving_density_list;

    initialize(capture.get(cv::CAP_PROP_FRAME_COUNT));
    calc_density(queue_density_list, moving_density_list, capture, frameRate, source_pts);
    // save(imagePath);
    make_graph(queue_density_list, moving_density_list, imagePath, frameRate);
}
// main function
int main(int argc, char **argv)
{
    frameRate = 5;
    videoPath = "trafficvideo.mp4";
    imagePath = "output.png";
    bool choose = false;

    parse(argc, argv, imagePath, videoPath, frameRate, choose);

    if (choose)
    {
        getCustomPoints(start);
    }
    else
    {
        start();
    }

    return 0;
}

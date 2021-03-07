#include <iostream>
#include <cmath>

#include "arg_parser.hpp"
#include "image_operations.hpp"
#include "density.hpp"
#include "graphs.hpp"

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
// main function
int main(int argc, char **argv)
{
    int frameRate = 15;
    string videoPath = "trafficvideo.mp4";
    string imagePath = "output.png";

    parse(argc, argv, imagePath, videoPath, frameRate);

    cv::VideoCapture capture = getImageStream(videoPath);
    vector<double> queue_density_list, moving_density_list;

    calc_density(queue_density_list, moving_density_list, capture, frameRate);

    make_graph(queue_density_list, moving_density_list, imagePath, frameRate);

    return 0;
}

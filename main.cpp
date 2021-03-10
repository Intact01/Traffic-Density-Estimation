#include <iostream>
#include <cmath>
#include <fstream>

#include "arg_parser.hpp"
#include "image_operations.hpp"
#include "density.hpp"
#include "choice.hpp"

int frameskip;
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
    cv::VideoCapture capture = getImageStream(videoPath);
    vector<double> queue_density_list, moving_density_list;

    initialize(capture.get(cv::CAP_PROP_FRAME_COUNT));
    calc_density(queue_density_list, moving_density_list, capture, frameskip, source_pts);
    make_graph(queue_density_list, moving_density_list, imagePath, frameskip);
}
bool hasEnding(std::string const &fileName, std::string const &extension)
{
    if (fileName.length() >= extension.length())
    {
        return (0 == fileName.compare(fileName.length() - extension.length(), extension.length(), extension));
    }
    else
    {
        return false;
    }
}
// main function
int main(int argc, char **argv)
{
    frameskip = 5;
    videoPath = "trafficvideo.mp4";
    imagePath = "output.png";
    bool choose = false;

    parse(argc, argv, imagePath, videoPath, frameskip, choose);

    std::ifstream file(videoPath);
    if (!file.is_open())
    {
        std::cout << "File not found. Aborting" << std::endl;
        return -1;
    }
    if (!(hasEnding(videoPath, ".avi") || hasEnding(videoPath, ".mp4") || hasEnding(videoPath, ".m4u") || hasEnding(videoPath, ".mkv")))
    {
        std::cout << "Invalid File. Aborting" << std::endl;
        return -1;
    }

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

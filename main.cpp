#include <iostream>
#include <algorithm>
#include <cmath>

#include "opencv2/opencv.hpp"
#include "arg_parser.hpp"
#include "image_operations.hpp"
#include "density.hpp"

using namespace std;

int counter = 0;
cv::Scalar green = cv::Scalar(0, 255, 0);
// vector_point scr_pts{Pt(805, 215), Pt(35, 836), Pt(1482, 825), Pt(1277, 212)};
// vector_point dest_pts{Pt(472, 52), Pt(472, 830), Pt(800, 52), Pt(800, 830)};
// function declarations

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
    // pair<string, bool> options; //image path, custom

    // options = parse(argc, argv);
    // string imageName = options.first; // image path

    string video_path = "trafficvideo.mp4";
    string imageName = "output.jpg";

    cv::Mat image = cv::imread(imageName, cv::IMREAD_GRAYSCALE);

    cv::VideoCapture capture = getImageStream(video_path);
    queue_density(capture, image, 5);

    return 0;
}

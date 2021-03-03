#include <iostream>
#include "opencv2/opencv.hpp"
#include "image_operations.hpp"
#include "matplotlibcpp.h"
#include <cmath>

namespace plt = matplotlibcpp;

using namespace std;
using namespace cv;

Mat processImage(Mat original_image)
{
    vector<Point2f> source_points; // points clicked by user
    vector<Point2f> destination_points;

    source_points.push_back(Point2f(762, 212));
    source_points.push_back(Point2f(3, 8813));
    source_points.push_back(Point2f(1277, 210));
    source_points.push_back(Point2f(1485, 866));
    

    destination_points.push_back(cv::Point2f(472, 52));
    destination_points.push_back(cv::Point2f(472, 830));
    destination_points.push_back(cv::Point2f(800, 52));
    destination_points.push_back(cv::Point2f(800, 830));


    // calculate perspective transformation matrix
    cv::Mat homography_matrix = cv::findHomography(source_points, destination_points);

    cv::Mat output_image;            //Transformed image

    //Transform perpective of original_image to output_image and store output_image to device
    try
    {
        cv::warpPerspective(original_image, output_image, homography_matrix, original_image.size());
    }
    catch (cv::Exception e)
    {
        cout << e.what() << endl;
        return output_image;
    }

    cv::Rect cropRect = getRectFromPoints(destination_points);
    cv::Mat cropped_image = output_image(cropRect);
    return cropped_image;
}
void make_graph(vector<double> moving_density_list){
    int n = 5000;
    vector<double> time;
    
    std::vector<double> x(n), y(n), z(n), w(n,2);

    for(int i=0; i<moving_density_list.size(); ++i) {
        // x.at(i) = i*i;
        // y.at(i) = sin(2*M_PI*i/360.0);
        // z.at(i) = log(i);
        time.push_back(i);
    }

    // Set the size of output image to 1200x780 pixels
    plt::figure_size(1200, 780);
    // Plot line from given x and y data. Color is selected automatically.
    plt::named_plot("Moving Density",time, moving_density_list);
    // Plot a red dashed line from given x and y data.
    // plt::plot(x, w,"r--");
    // // Plot a line whose name will show up as "log(x)" in the legend.
    // plt::named_plot("log(x)", x, z);
    // Set x-axis to interval [0,1000000]
    plt::xlim(0, 400);
    // Add graph title
    plt::title("Traffic Density Plot");
    // Enable legend.
    plt::legend();
    // Save the image (file format is determined by the extension)
    plt::save("./basic.png");
}
int main()
{
    VideoCapture capture("trafficvideo.mp4");
    bool to_gray = true, save = false;
    int counter = 0;

    if (!capture.isOpened())
    {
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
    }
    Mat frame1, prvs;
    capture >> frame1;


    // Preprocessing for exact method
    frame1 = processImage(frame1);
    cvtColor(frame1, prvs, COLOR_BGR2GRAY);
    vector<double> moving_density_list;
    int total_pixels = frame1.rows * frame1.cols;

    while (true)
    {
        counter++;

        // Read the next frame
        Mat frame2, next;
        capture >> frame2;
        if (frame2.empty())
            break;

        if (counter % 5 != 0) continue;

        frame2 = processImage(frame2);

        // Preprocessing for exact method
        cvtColor(frame2, next, COLOR_BGR2GRAY);
        
        // Calculate Optical Flow
        Mat flow(prvs.size(), CV_32FC2);
        calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
        // method(prvs, next, flow, std::forward<Args>(args)...);
        int changed_pixels = 0;
        int rows = flow.rows;
        int columns = flow.cols;
        
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < columns; j++){
                if (flow.at<double>(i,j) > 0.00001){
                    changed_pixels ++;
                }
            }
        }
        
        moving_density_list.push_back((double)changed_pixels/(double)total_pixels);
        // Visualization part
        Mat flow_parts[2];
        split(flow, flow_parts);

        // Convert the algorithm's output into Polar coordinates
        Mat magnitude, angle, magn_norm;

        cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
       

        

        
        // Update the previous frame
        prvs = next;
        
    }
    cout<<"Voila"<<endl;
    make_graph(moving_density_list);
    return 0;
}
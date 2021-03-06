#include <iostream>
#include "opencv2/opencv.hpp"
#include "image_operations.hpp"
#include "matplotlibcpp.h"
#include <cmath>

namespace plt = matplotlibcpp;

using namespace std;
using namespace cv;


int cmp_x(const cv::Point2f &lhs, const cv::Point2f &rhs)
{
    return lhs.x < rhs.x;
}
int cmp_y(const cv::Point2f &lhs, const cv::Point2f &rhs)
{
    return lhs.y < rhs.y;
}
Mat processImage(Mat original_image)
{
    vector<Point2f> source_points; // points clicked by user
    vector<Point2f> destination_points;

    source_points.push_back(Point2f(1258, 164));
    source_points.push_back(Point2f(1484, 833));
    source_points.push_back(Point2f(138, 792));
    source_points.push_back(Point2f(913, 140));

    stable_sort(source_points.begin(), source_points.end(), cmp_x);
    stable_sort(source_points.begin(), source_points.begin() + 2, cmp_y);
    stable_sort(source_points.begin() + 2, source_points.begin() + 4, cmp_y);
    

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
    vector<double> mdl;
    
    for(int i=0; i<moving_density_list.size(); ++i) {
        if (i == 0){
            moving_density_list[0] = (moving_density_list[0] + moving_density_list[1] + moving_density_list[2]) / 3;
        }
        else if(i==1){
            moving_density_list[1] = (moving_density_list[0] + moving_density_list[1] + moving_density_list[2] + moving_density_list[3]) / 4;
        }
        else if(i == moving_density_list.size() -1){
            moving_density_list[i] = (moving_density_list[i-1] + moving_density_list[i]) + moving_density_list[i-2] /2;
        }
        else if(i==moving_density_list.size() -2){
            moving_density_list[i] = (moving_density_list[i-1] + moving_density_list[i] + moving_density_list[i+1] + moving_density_list[i-2]) /4;
        }
        else{
            moving_density_list[i] = (moving_density_list[i-2]+moving_density_list[i-1]+ moving_density_list[i] + moving_density_list[i+1] + moving_density_list[i+2]) /5;
        }
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
    int k = moving_density_list.size();
    plt::xlim(0,k);
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

        if (counter % 15 != 0) continue;

        frame2 = processImage(frame2);

        // Preprocessing for exact method
        cvtColor(frame2, next, COLOR_BGR2GRAY);

        Mat flow(prvs.size(), CV_32FC2);
        calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
        
        
        // Visualization part
        Mat flow_parts[2];
        split(flow, flow_parts);
        
        // Convert the algorithm's output into Polar coordinates
        Mat magnitude, angle, magn_norm;


        cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
        normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
        angle *= ((1.f / 360.f) * (180.f / 255.f));

        Mat _hsv[3], hsv, hsv8, bgr, grey, thresh;

        _hsv[0] = angle;
        _hsv[1] = Mat::ones(angle.size(), CV_32F);
        _hsv[2] = magn_norm;
        merge(_hsv, 3, hsv);
        hsv.convertTo(hsv8, CV_8U, 255.0);
        cvtColor(hsv8, bgr, COLOR_HSV2BGR);
        cvtColor(bgr, grey, COLOR_BGR2GRAY);
        threshold(grey, thresh, 40, 255, THRESH_BINARY);
        // imshow("flow", bgr);
        // imshow("grey", grey);
        // imshow("frame", frame2);
        // imshow("thresh", thresh);
        // waitKey(30);
        int changed_pixels = countNonZero(thresh);
        
        moving_density_list.push_back((double)changed_pixels/(double)total_pixels);
        

        
        // Update the previous frame
        prvs = next;
        cout<<counter<<endl;
    }
    destroyAllWindows();
    cout<<"Voila"<<endl;
    make_graph(moving_density_list);
    return 0;
}









// int changed_pixels = 0;
        // int rows = flow.rows;
        // int columns = flow.cols;
        
        // for(int i = 0; i < rows; i++){
        //     for(int j = 0; j < columns; j++){
        //         if (flow.at<double>(i,j) > 0.0001){
        //             changed_pixels ++;
        //         }
        //     }
        // }
        
        // moving_density_list.push_back((double)changed_pixels/(double)total_pixels);

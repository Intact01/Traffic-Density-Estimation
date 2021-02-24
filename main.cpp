#include <iostream>
#include <algorithm>

#include "opencv2/opencv.hpp"
#include "arg_parser.hpp"
#include "image_operations.hpp"

using namespace std;
int counter = 0;
cv::Mat source_image;   //input image with guide lines and mouse-click points
cv::Mat cache_image;    //input image with mouse point tracking line
cv::Mat original_image; //input image
cv::Scalar green = cv::Scalar(0, 255, 0);
vector<cv::Point2f> source_points;      //points clicked by user
vector<cv::Point2f> destination_points; //Points in the transformed image corresponding to the source_points

//function declarations
void mouse_callback(int event, int x, int y, int flag, void *param);

//comparator functions used in sorting
int cmp_x(const cv::Point2f &lhs, const cv::Point2f &rhs)
{
    return lhs.x < rhs.x;
}
int cmp_y(const cv::Point2f &lhs, const cv::Point2f &rhs)
{
    return lhs.y < rhs.y;
}

//Tranformes perspective of original image
//Stores transformed image
void processImage()
{
    //sort source_points
    stable_sort(source_points.begin(), source_points.end(), cmp_x);
    stable_sort(source_points.begin(), source_points.begin() + 2, cmp_y);
    stable_sort(source_points.begin() + 2, source_points.begin() + 4, cmp_y);

    //sort destination_points
    stable_sort(destination_points.begin(), destination_points.end(), cmp_x);
    stable_sort(destination_points.begin(), destination_points.begin() + 2, cmp_y);
    stable_sort(destination_points.begin() + 2, destination_points.begin() + 4, cmp_y);

    //calculate perspective transformation matrix
    cv::Mat homography_matrix = cv::findHomography(source_points, destination_points);

    cv::Mat output_image;            //Transformed image
    cv::Mat output_image_with_lines; //Transformed image with lines displaying crop-area

    //Transform perpective of original_image to output_image and store output_image to device
    try{
    cv::warpPerspective(original_image, output_image, homography_matrix, source_image.size());
    }catch(cv::Exception e){
        cout << e.what()<<endl;
        return;
    }
    

    output_image.copyTo(output_image_with_lines);
    cv::destroyWindow("Source Image");

    // Draw points and lines on output_image_with_lines
    int order[]{0, 1, 3, 2, 0};
    for (int i = 0; i < 4; i++)
    {
        cv::circle(output_image_with_lines, destination_points[i], 5, green, 2, 8, 0);
        cv::line(output_image_with_lines, destination_points[order[i]], destination_points[order[i + 1]], green, 1, 8, 0);
    }

    // Display message
    string helpText = "Press any key to confirm, Esc. to cancel";
    cv::Point textPos = cv::Point2f(destination_points[0].x - 100, (destination_points[0].y + destination_points[1].y) / 2);
    cv::putText(output_image_with_lines, helpText, textPos, cv::FONT_HERSHEY_PLAIN, 2.0, green, 2, true);

    //show output_image_with_lines
    cv::imshow("Transformed Image", output_image_with_lines);
    cout << helpText << endl;

    //wait for user to press a key
    int key = cv::waitKey(0);

    //if key is escape => cancel else crop the transformed image
    if (key == 27)
    {
        exit(-1);
    }
    cv::destroyWindow("Transformed Image");
    cv::imwrite("transformed.jpg", output_image);
    cv::Mat cropped_image = crop_image(output_image, destination_points);
    cv::imshow("Cropped Image", cropped_image);
    cv::imwrite("cropped.jpg", cropped_image);
    cv::waitKey(0);
}

// tracks mouse events
void mouse_callback(int event, int x, int y, int flag, void *param)
{
    cv::Point2f newpt = cv::Point2f(x, y);

    // left click
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        counter++;

        //till 4 points are selected
        if (counter <= 4)
        {
            source_points.push_back(newpt);

            //display circle at clicked point
            cv::circle(source_image, newpt, 5, green, 2, 8, 0);
            cv::imshow("Source Image", source_image);
        }
        if (counter > 1 && counter <= 4)
        {
            // join clicked point with previously clicked point
            cv::line(source_image, source_points[counter - 2], source_points[counter - 1], green, 1, 8, 0);
            cv::imshow("Source Image", source_image);
        }

        cout << x << " " << y << endl;
        if (counter == 4)
        {
            // complete the rectangle
            cv::line(source_image, source_points[0], source_points[counter - 1], green, 1, 8, 0);
            cv::imshow("Source Image", source_image);
            processImage();
        }
    }

    // mouse pointer tracking
    else if (event == cv::EVENT_MOUSEMOVE)
    {
        //until 4 points are selected
        if (counter > 0 && counter < 4)
        {
            // show mouse pointer by a line joined to previously clicked point
            source_image.copyTo(cache_image);
            cv::line(cache_image, source_points[counter - 1], newpt, green, 1, 8, 0);
            cv::imshow("Source Image", cache_image);
        }
    }
}

// initializing destination points
void initialize(bool custom_input)
{
    if (custom_input)
    {
        cout << "Please enter 4 destination points." << endl;
        for (int i = 1; i < 5; i++)
        {
            cout << "Enter space-separated coordinates of point " << i <<":"<< endl;
            int temp_x, temp_y;
            cin >> temp_x >> temp_y;
            destination_points.push_back(cv::Point2f(temp_x, temp_y));
        }
    }
    else
    {
        destination_points.push_back(cv::Point2f(472, 52));
        destination_points.push_back(cv::Point2f(472, 830));
        destination_points.push_back(cv::Point2f(800, 52));
        destination_points.push_back(cv::Point2f(800, 830));
    }
}

// main function
int main(int argc, char **argv)
{
    pair<string, bool> options; //image path/name

    options = parse(argc, argv);
    string imageName = options.first;

    if (imageName == "")
        return -1;

    initialize(options.second);

    source_image = cv::imread(imageName, cv::IMREAD_GRAYSCALE);

    //wrong image name
    if (source_image.empty())
    {
        cout << "Could not open or find the image : " << imageName << endl;
        return -1;
    }

    cache_image = source_image.clone();
    original_image = cache_image.clone();

    //display input image
    cv::imshow("Source Image", source_image);

    cv::setMouseCallback("Source Image", mouse_callback);
    cv::waitKey(0);
    return 0;
}

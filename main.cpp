#include "opencv2/opencv.hpp"
#include <iostream>
#include <algorithm>
#include <vector>


int counter = 0;
cv::Mat source_image;
cv::Mat cache_image;
cv::Mat original_image;
cv::Scalar green = cv::Scalar(0, 255, 0);

std::vector<cv::Point2f> source_points;


void mouse_callback(int event, int x, int y, int flag, void *param);
void crop_image(cv::Mat, std::vector<cv::Point2f> points);

int cmp_x(const cv::Point2f &lhs, const cv::Point2f &rhs) {
      return lhs.x < rhs.x;
}
int cmp_y(const cv::Point2f &lhs, const cv::Point2f &rhs) {
      return lhs.y < rhs.y;
}

void processImage(){
    std::vector<cv::Point2f> destination_points;
    destination_points.push_back(cv::Point2f(472,52));
    destination_points.push_back(cv::Point2f(472,830));
    destination_points.push_back(cv::Point2f(800,52));
    destination_points.push_back(cv::Point2f(800,830));

    std::stable_sort(source_points.begin(), source_points.end(), cmp_x);
    std::stable_sort(source_points.begin(), source_points.begin() + 2, cmp_y);
    std::stable_sort(source_points.begin()+2, source_points.begin() + 4, cmp_y);


    cv::Mat homography_matrix = cv::findHomography(source_points, destination_points);

    cv::Mat output_image;
    cv::warpPerspective(original_image, output_image, homography_matrix, source_image.size());    

    cv::imwrite("transformed.jpg",output_image);
    cv::imshow("Transformed Image", output_image);
    crop_image(output_image, destination_points);
    cv::waitKey(0); 
}

void crop_image(cv::Mat img, std::vector<cv::Point2f> points){

    cv::Rect crop_area;
    
    crop_area.width = points[3].x - points[0].x;
    crop_area.height = points[1].y - points[0].y;
    crop_area.x = points[0].x;
    crop_area.y = points[0].y;

    cv::Mat cropped_image = img(crop_area);
    cv::imshow("Cropped Image", cropped_image);
    cv::imwrite("cropped.jpg",cropped_image);
}


void mouse_callback(int event, int x, int y, int flag, void *param)
{
    cv::Point2f newpt = cv::Point2f(x, y);
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        source_points.push_back(newpt);
        counter++;

        if (counter <= 4)
        {
            cv::circle(source_image, newpt, 5, green, 2, 8, 0);
            cv::imshow("Source Image", source_image);
        }
        if (counter > 1 && counter <= 4)
        {
            cv::line(source_image, source_points[counter - 2], source_points[counter - 1], green, 1, 8, 0);
            cv::imshow("Source Image", source_image);
        }

        std::cout << x << " " << y << std::endl;
        if (counter == 4)
        {
            cv::line(source_image, source_points[0], source_points[counter - 1], green, 1, 8, 0);
            cv::imshow("Source Image", source_image);
            processImage();
        }
    }
    else if (event == cv::EVENT_MOUSEMOVE)
    {
        if (counter > 0 && counter < 4)
        {
            source_image.copyTo(cache_image);
            cv::line(cache_image, source_points[counter - 1], newpt, green, 1, 8, 0);
            cv::imshow("Source Image", cache_image);
        }
    }
}

int main(int argc, char **argv)
{
    std::string image_name;

    if (argc == 2)
    {
        image_name = argv[1];
    }
    else
    {
        std::cout << "Please enter ONE argument : the name of the image file"
                  << "\n";
        return -1;
    }
    source_image = cv::imread(image_name, cv::IMREAD_GRAYSCALE);
    if (source_image.empty())
    {
        std::cout << "Could not open or find the image : "<<image_name << std::endl;
        return -1;
    }
    cache_image = source_image.clone();
    original_image = cache_image.clone();
    cv::imshow("Source Image", source_image);

    cv::setMouseCallback("Source Image", mouse_callback);
    cv::waitKey(0);
    return 0;
}

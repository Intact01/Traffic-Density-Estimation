#include "opencv2/opencv.hpp"
#include "arg_parser.hpp"
#include <iostream>
#include <algorithm>

using namespace std;

int counter = 0;
cv::Mat source_image;
cv::Mat cache_image;
cv::Mat original_image;
cv::Scalar green = cv::Scalar(0, 255, 0);

vector<cv::Point2f> source_points;

void mouse_callback(int event, int x, int y, int flag, void *param);
void crop_image(cv::Mat, vector<cv::Point2f> points);

int cmp_x(const cv::Point2f &lhs, const cv::Point2f &rhs)
{
    return lhs.x < rhs.x;
}
int cmp_y(const cv::Point2f &lhs, const cv::Point2f &rhs)
{
    return lhs.y < rhs.y;
}

void processImage()
{
    vector<cv::Point2f> destination_points;
    destination_points.push_back(cv::Point2f(472, 52));
    destination_points.push_back(cv::Point2f(472, 830));
    destination_points.push_back(cv::Point2f(800, 52));
    destination_points.push_back(cv::Point2f(800, 830));

    stable_sort(source_points.begin(), source_points.end(), cmp_x);
    stable_sort(source_points.begin(), source_points.begin() + 2, cmp_y);
    stable_sort(source_points.begin() + 2, source_points.begin() + 4, cmp_y);

    cv::Mat homography_matrix = cv::findHomography(source_points, destination_points);

    cv::Mat output_image;
    cv::Mat output_image_with_lines;
    cv::warpPerspective(original_image, output_image, homography_matrix, source_image.size());
    output_image.copyTo(output_image_with_lines);

    cv::destroyWindow("Source Image");

    // Draw points and lines on output image

    int order[]{0, 1, 3, 2, 0};
    for (int i = 0; i < 4; i++)
    {
        cv::circle(output_image_with_lines, destination_points[i], 5, green, 2, 8, 0);
        cv::line(output_image_with_lines, destination_points[order[i]], destination_points[order[i + 1]], green, 1, 8, 0);
    }

    string helpText = "Press any key to crop, Esc. to cancel";
    cv::Point textPos = cv::Point2f(destination_points[0].x - 100, (destination_points[0].y + destination_points[1].y) / 2);
    cv::putText(output_image_with_lines, helpText, textPos, cv::FONT_HERSHEY_PLAIN, 2.0, green, 2, true);
    cv::imshow("Transformed Image", output_image_with_lines);
    cout << helpText << endl;

    int key = cv::waitKey(0);
    if (key == 27)
    {
        exit(-1);
    }

    cv::imwrite("transformed.jpg", output_image);
    cv::destroyWindow("Transformed Image");

    crop_image(output_image, destination_points);
}

void crop_image(cv::Mat img, vector<cv::Point2f> points)
{

    cv::Rect crop_area;

    crop_area.width = points[3].x - points[0].x;
    crop_area.height = points[1].y - points[0].y;
    crop_area.x = points[0].x;
    crop_area.y = points[0].y;

    cv::Mat cropped_image = img(crop_area);
    cv::imshow("Cropped Image", cropped_image);
    cv::imwrite("cropped.jpg", cropped_image);
    cv::waitKey(0);
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

        cout << x << " " << y << endl;
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
    string image_name;

    image_name = parse(argc,argv);
    if (image_name == "") return -1;
    source_image = cv::imread(image_name, cv::IMREAD_GRAYSCALE);
    if (source_image.empty())
    {
        cout << "Could not open or find the image : " << image_name << endl;
        return -1;
    }
    cache_image = source_image.clone();
    original_image = cache_image.clone();
    cv::imshow("Source Image", source_image);

    cv::setMouseCallback("Source Image", mouse_callback);
    cv::waitKey(0);
    return 0;
}

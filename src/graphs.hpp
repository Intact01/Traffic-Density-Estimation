#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/plot.hpp>

#include "matplotlibcpp.h"
#include "properties.hpp"

using namespace std;
namespace plt = matplotlibcpp;

cv::Mat plot_result, x_data, y_data_queue, y_data_moving;
cv::Ptr<cv::plot::Plot2d> plot;
vector<double> time_list;
int array_index = 0;

int initialize(int num_frames) {
  int time_in_sec = num_frames / 15;
  for (int i = 0; i < time_in_sec; ++i) {
    time_list.push_back(i);
  }
  x_data = cv::Mat(1, num_frames, CV_64F);
  y_data_queue = cv::Mat(1, num_frames, CV_64F);
  y_data_moving = cv::Mat(1, num_frames, CV_64F);

  for (int i = 0; i < x_data.cols; i++) {
    x_data.at<double>(0, i) = 0;
    y_data_queue.at<double>(0, i) = 0;
    y_data_moving.at<double>(0, i) = 0;
  }
  // time = cv::Mat(1, time_in_sec, CV_64F);
  // plot = cv::plot::Plot2d::create(x_data, y_data);

  return 0;
}

void make_graph(vector<double> queue_density_list,
                vector<double> moving_density_list, string &fileName,
                int frameskip) {
  vector<double> time;
  for (int i = 0; i < queue_density_list.size(); ++i) {
    time.push_back(i * frameskip / 15);
  }
  // Set the size of output image to 1200x780 pixels
  plt::figure_size(1200, 780);
  plt::xlabel("Time");
  plt::ylabel("Density of Cars");
  // Plot line from given x and y data. Color is selected automatically.
  if (queue_density_list.size() > 0)
    plt::named_plot("Queue Density", time, queue_density_list);

  if (moving_density_list.size() > 0)
    plt::named_plot("Moving Density", time, moving_density_list);

  int xlim_val =
      (int)((max(queue_density_list.size(), moving_density_list.size())) *
            frameskip / 15);
  plt::xlim(0, xlim_val);
  // Add graph title
  plt::title("Traffic Density Plot");
  // Enable legend.
  plt::legend();
  // Save the image (file format is determined by the extension)
  plt::save(fileName);
}

// void update(vector<double> queue_density, vector<double> moving_density)
void update(double queue_density, double moving_density) {
  try {
    // cv::Mat x_data(time_list), y_data(queue_density);
    x_data.at<double>(0, array_index) = array_index++;
    y_data_queue.at<double>(0, array_index) = queue_density;
    y_data_moving.at<double>(0, array_index) = moving_density;

    plot = cv::plot::Plot2d::create(x_data, y_data_queue);

    plot->setPlotLineColor(CYAN);
    plot->setPlotLineWidth(1.5);
    plot->setShowText(false);
    plot->setShowGrid(false);
    plot->setInvertOrientation(true);
    plot->render(plot_result);

    cv::imshow("Queue Density", plot_result);
    cv::moveWindow("Queue Density", 150, 400);

    plot = cv::plot::Plot2d::create(x_data, y_data_moving);

    plot->setPlotLineColor(ORANGE);
    plot->setPlotLineWidth(1.5);
    plot->setShowText(false);
    plot->setShowGrid(false);
    plot->setInvertOrientation(true);
    plot->render(plot_result);

    cv::imshow("Dynamic Density", plot_result);
    cv::moveWindow("Dynamic Density", 1200, 400);

    cv::waitKey(30);
  } catch (cv::Exception e) {
    std::cout << e.what() << endl;
  }
}
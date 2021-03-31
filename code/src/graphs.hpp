#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/plot.hpp>

#include "matplotlibcpp.h"
#include "properties.hpp"

using namespace std;
namespace plt = matplotlibcpp;

void make_graph(vector<double> queue_density_list,
                vector<double> moving_density_list, string &fileName,
                int frameskip) {
  vector<double> time;
  int list_size = max(queue_density_list.size(), moving_density_list.size());

  for (int i = 0; i < list_size; ++i) {
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

  int xlim_val = (int)(list_size * frameskip / 15);

  // Set limits for the X-axis
  plt::xlim(0, xlim_val);
  plt::title("Traffic Density Plot");
  plt::legend();

  // Save the image (file format is determined by the extension)
  plt::save(fileName);
}
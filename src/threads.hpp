// #pragma once
// #include <bits/stdc++.h>

// #include "density.hpp"
// #include "graphs.hpp"
// #include "image_operations.hpp"
// #include "opencv2/opencv.hpp"
// #include "properties.hpp"

// struct Method4ThreadArgs {
//   vector<double> &queue_density_list, &moving_density_list;
//   cv::VideoCapture capture;
//   int fast_forward;
//   vector_point source_points, threading_crop_points;
// };

// void calc_density_threaded(void *args) {
//   Method4ThreadArgs *arguments = (struct Method4ThreadArgs *)args;
//   calc_density(arguments->queue_density_list, arguments->moving_density_list,
//                arguments->capture, arguments->fast_forward,
//                arguments->source_points, arguments->threading_crop_points);
// }
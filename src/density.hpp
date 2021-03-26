#pragma once
#include <iostream>
#include<pthread.h>
#include <opencv2/ximgproc/sparse_match_interpolator.hpp>
#include "image_operations.hpp"
#include "properties.hpp"
#include "graphs.hpp"

#define NUM_THREADS 5
typedef cv::Ptr<cv::BackgroundSubtractor> bagSub;

vector<double> qd_list;
vector_point src_pts;
int total_pixels = -1;
vector<cv::Mat> frames;
bagSub pBSub;
// struct thread4_args{
//   bagSub pBackSub;
//   int rem;
// };
// calculate queue density
int processQueue(cv::Mat frame, bagSub pBackSub) {
  cv::Mat frame1, prvs, fgMask;

  // create the structuring element that can be further passed to erode and
  // dilate
  int dilation_size = 2;
  cv::Mat element2 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
      cv::Point(dilation_size, dilation_size));
  pBackSub->apply(frame, fgMask, 0);
  fgMask.copyTo(frame1);

  // successive erosion and dilation to fill holes in parse
  cv::Mat thresh;
  cv::erode(fgMask, thresh, element2);
  cv::dilate(frame1, frame1, element2);
  cv::erode(fgMask, thresh, element2);
  cv::dilate(frame1, frame1, element2);

  cv::dilate(frame1, frame1, element2);

  // count pixels only greater than 127
  cv::threshold(frame1, frame1, 127, 255, cv::THRESH_BINARY);
  int val = cv::countNonZero(frame1);

  cv::waitKey(30);
  return val;
}

// calculate motion density with optical flow
int processMotion(cv::Mat frame, cv::Mat prvs, cv::Mat &next) {
  // copy frame to next
  frame.copyTo(next);

  // create the structuring element that can be further passed to erode
  int erosion_size = 2;
  cv::Mat element1 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
      cv::Point(erosion_size, erosion_size));
  // calculate optical flow
  cv::Mat flow(prvs.size(), CV_32FC2);
  cv::calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

  cv::Mat flow_parts[2];
  split(flow, flow_parts);

  // Convert the output into Polar coordinates
  cv::Mat magnitude, angle, magn_norm;
  cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
  cv::normalize(magnitude, magn_norm, 0.0f, 1.0f, cv::NORM_MINMAX);
  angle *= ((1.f / 360.f) * (180.f / 255.f));

  // create image of flow
  cv::Mat _hsv[3], hsv, hsv2, bgr, gray, thresh;
  _hsv[0] = angle;
  _hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
  _hsv[2] = magn_norm;
  cv::merge(_hsv, 3, hsv);
  hsv.convertTo(hsv2, CV_8U, 255.0);

  // convert hsv to gray
  cv::cvtColor(hsv2, bgr, cv::COLOR_HSV2BGR);
  cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

  // keep pixels only greater than 127
  cv::threshold(gray, thresh, 127, 255, cv::THRESH_TRIANGLE);

  // errode and dilate to fill the holes
  cv::erode(thresh, thresh, element1);
  cv::dilate(thresh, thresh, element1);

  // count nonzero pixels
  int changed_pixels = cv::countNonZero(thresh);
  return changed_pixels;
}

// for density calculation
void calc_density(vector<double> &queue_density_list,
                  vector<double> &moving_density_list, cv::VideoCapture capture,
                  int fast_forward, vector_point source_points) {
  cv::Mat frame, prvs, next;
  capture >> frame;  // capture the first frame

  vector<double> original_moving_list;

  int counter = 0;
  int index = 0;

  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  prvs = cameraCorrection(frame, source_points, dest_pts);

  int total_pixels = prvs.rows * prvs.cols;

  bagSub pBackSub1, pBackSub2;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor
  pBackSub2 = cv::createBackgroundSubtractorKNN(
      40);  // to create background subtractor using starting 40 frames

  double last_k_sum = 0;
  int k = 5;

  while (true) {
    counter++;
    capture >> frame;  // capture next frame

    if (counter % fast_forward != 0) continue;
    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);

    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    // update original motion density list
    int changed_pixels = processMotion(frame, prvs, next);
    double moving_density = (double)changed_pixels / total_pixels;
    original_moving_list.push_back(moving_density);
    index++;

    // take avg with last k frames and update motion density list
    last_k_sum += moving_density;
    if (index > k) last_k_sum -= original_moving_list[index - k];

    double actual_density = last_k_sum / k;
    moving_density_list.push_back(actual_density);

    // show image frame
    imshow("Frame", frame);

    //  Update the previous frame
    frame.copyTo(prvs);
    std::cout << left << setw(10) << double(counter) / 15 << left << setw(10)
              << queue_density << left << setw(10) << moving_density << endl;

    // update graph
    update(queue_density, actual_density);
  }
  std::cout << "Generating final graph... Please Wait" << endl;
}


void *threading_frames(void *arguments){
  // struct thread4_args *args = (struct thread4_args *)arguments;
  // int rem = args -> rem;
  // cout<<rem<<endl;
  // bagSub pBackSub = args -> pBackSub;
  cv::Mat frame;
  int* rem = (int *)arguments;
  // while(cap.grab()){
  //   cout<<"grabbed"<<endl;
  //   if ((int) cap.get(cv::CAP_PROP_POS_FRAMES) % NUM_THREADS == rem && cap.get(cv::CAP_PROP_POS_FRAMES) != 0){
  //     cout<<cap.get(cv::CAP_PROP_POS_FRAMES)<<endl;
  //     cap.retrieve(frame);
  //     cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  //     frame = cameraCorrection(frame, src_pts, dest_pts);
  //     int queueSum = processQueue(frame, pBackSub);
  //     if(total_pixels == -1){
  //       total_pixels = frame.rows * frame.cols;
  //     }
  //     double queue_density = (double)queueSum / total_pixels;
  //     qd_list[cap.get(cv::CAP_PROP_POS_FRAMES)-1] = queue_density;
  //   }
  // }
  for(int i = *rem; i<frames.size(); i+=NUM_THREADS){
    frames[i].copyTo(frame);
    // imshow("thread "+to_string(*rem), frame);
    // cv::waitKey(10);
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, src_pts, dest_pts);
    int queueSum = processQueue(frame, pBSub);
    if(total_pixels == -1){
      total_pixels = frame.rows * frame.cols;
    }
    double queue_density = (double)queueSum / total_pixels;
    qd_list[i] = queue_density;
    cout<<i<<" "<<qd_list[i]<<endl;
  }
  return NULL;
}

void method4(vector<double> &queue_density_list, cv::VideoCapture capture, vector_point source_points){
  pthread_t threads[NUM_THREADS];
  pthread_attr_t attr;
  // void *status;
  // pthread_attr_init(&attr);
  // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int td[NUM_THREADS];
  pBSub =
      cv::createBackgroundSubtractorMOG2(); 
  qd_list = queue_density_list;
  qd_list.resize(capture.get(cv::CAP_PROP_FRAME_COUNT) -1);
  src_pts = source_points;
  cv::Mat frame;
  capture.set(cv::CAP_PROP_FORMAT, CV_32F);
  capture>>frame;
  int counter =0;
  while(true){
    capture>>frame;
    
    if(frame.empty()){
      break;
    }
    // if(counter<500){
    //   cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    //   frame = cameraCorrection(frame, src_pts, dest_pts);
    //   int queueSum = processQueue(frame, pBSub);
    //   if(total_pixels == -1){
    //     total_pixels = frame.rows * frame.cols;
    //   }
    //   double queue_density = (double)queueSum / total_pixels;
    //   qd_list[counter] = queue_density;
    //   cout<<counter<<" "<<qd_list[counter]<<endl;
    //   counter++;
    // }
    frames.push_back(frame);
  }
  for (int i=0; i<NUM_THREADS; i++){
    // td[i].pBackSub = pBackSub1;
    // td[i].rem = i;
    td[i] =i;
    cout<<"create thread "<<i<<endl;
    int rc = pthread_create(&threads[i], NULL, threading_frames, (void *)&td[i]);
    if (rc) {
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
  }
  for (int i=0; i<NUM_THREADS; i++){
    pthread_join(threads[i], NULL);
  }
  cout<<"done"<<endl;
  queue_density_list = qd_list;
  return;
}
void method0(vector<double> &queue_density_list, cv::VideoCapture capture, vector_point source_points, int fast_forward = 1){
  cv::Mat frame;
  capture >> frame;  // capture the first frame


  int counter = 0;
  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  cout<<total_pixels<<endl;
  bagSub pBackSub1;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor


  while (true) {
    counter++;
    capture >> frame;  // capture next frame

    if (counter % fast_forward != 0) continue;
    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);
    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    std::cout << left << setw(10) << (counter) << left << setw(10)
              << queue_density << left << endl;
  }
}

void method2(vector<double> &queue_density_list, cv::VideoCapture capture, vector_point source_points, int width=480 , int height = 360){
  cv::Mat frame;
  capture >> frame;  // capture the first frame

  int counter = 0;
  for(int i=0; i<4;i++){
    cv::Point2f pt;
    pt = dest_pts[i];
    pt.x = pt.x*width/frame.size().width;
    pt.y = pt.y*height/frame.size().height;
    dest_pts[i] = pt;

    pt = source_points[i];
    pt.x = pt.x*width/frame.size().width;
    pt.y = pt.y*height/frame.size().height;
    source_points[i] = pt;
  }
  resize(frame, frame, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);
  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  frame = cameraCorrection(frame, source_points, dest_pts);
  int total_pixels = frame.rows * frame.cols;
  cout<<total_pixels<<endl;
  bagSub pBackSub1;
  pBackSub1 =
      cv::createBackgroundSubtractorMOG2();  // to create background subtractor
  while (true) {
    counter++;
    capture >> frame;  // capture next frame

    if (frame.empty()) break;
    resize(frame, frame, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);
    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);
    // update queue density list
    int queueSum = processQueue(frame, pBackSub1);
    double queue_density = (double)queueSum / total_pixels;
    queue_density_list.push_back(queue_density);

    //std::cout << (counter) <<" "<< queue_density << endl;
  }
}

void method0_md(vector<double> &moving_density_list, cv::VideoCapture capture, vector_point source_points){
  cv::Mat frame, prvs, next;
  capture >> frame;  // capture the first frame

  vector<double> original_moving_list;

  int counter = 0;
  int index = 0;

  cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  prvs = cameraCorrection(frame, source_points, dest_pts);

  int total_pixels = prvs.rows * prvs.cols;

  double last_k_sum = 0;
  int k = 5;

  while (true) {
    counter++;
    capture >> frame;  // capture next frame

    if (frame.empty()) break;

    // convert to greyscale and correct the camera angle
    cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    frame = cameraCorrection(frame, source_points, dest_pts);

    // update original motion density list
    int changed_pixels = processMotion(frame, prvs, next);
    double moving_density = (double)changed_pixels / total_pixels;
    original_moving_list.push_back(moving_density);
    index++;

    // take avg with last k frames and update motion density list
    last_k_sum += moving_density;
    if (index > k) last_k_sum -= original_moving_list[index - k];

    double actual_density = last_k_sum / k;
    moving_density_list.push_back(actual_density);

    //  Update the previous frame
    frame.copyTo(prvs);
    std::cout << left << setw(10) << (counter)<< left << setw(10)
              << actual_density << endl;

  }

}

void method5(vector<double> &moving_density_list, cv::VideoCapture capture, vector_point source_points){

  cv::Mat prvs, prvs_gray;
  vector<cv::Point2f> p0, p1;
  // Take first frame and find corners in it
  capture >> prvs;
  prvs = cameraCorrection(prvs, source_points, dest_pts);
  int total_pixels = prvs.rows*prvs.cols;
  cvtColor(prvs, prvs_gray, cv::COLOR_BGR2GRAY);
  cv::goodFeaturesToTrack(prvs_gray, p0, 300, 0.1, 7, cv::Mat(), 7, false, 0.04);
  // Create a mask image for drawing purposes
  cv::Mat mask = cv::Mat::zeros(prvs.size(), prvs.type());
  int dilation_size = 3;
  cv::Mat element2 = cv::getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
      cv::Point(dilation_size, dilation_size));
  int counter =0;
  while(true){
    counter++;
    cv::Mat frame, frame_gray;
    capture >> frame;
    if (frame.empty())
        break;
    frame = cameraCorrection(frame, source_points, dest_pts);
    cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
    // calculate optical flow
    vector<uchar> status;
    vector<float> err;
    cv::TermCriteria criteria = cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 10, 0.03);
    cv::calcOpticalFlowPyrLK(prvs_gray, frame_gray, p0, p1, status, err, cv::Size(20,20), 2, criteria);
    mask = cv::Mat::zeros(frame.size(), frame.type());
    vector<cv::Point2f> good_new;
    for(uint i = 0; i < p0.size(); i++)
    {
        // Select good points
        if(status[i] == 1) {
            good_new.push_back(p1[i]);
            // draw the tracks
            if(dist(p1[i], p0[i])>=0.5){
              line(mask,p1[i], p0[i], WHITE, 5);
            }
            
            circle(frame, p1[i], 5, BLACK, -1);
        }
    }
    cv::dilate(mask, mask, element2);
    cv::dilate(mask, mask, element2);
    cv::dilate(mask, mask, element2);
    cv::dilate(mask, mask, element2);
    cv::dilate(mask, mask, element2);
    cv::dilate(mask, mask, element2);
    // imshow("frame", frame);
    cvtColor(mask,mask,cv::COLOR_BGR2GRAY);
    // imshow("mask", mask);
    double moving_desnity = (double)cv::countNonZero(mask)/total_pixels;
    moving_density_list.push_back(moving_desnity);
    std::cout<<left <<setw(10)<< counter<<left<<setw(10)<<moving_desnity<<endl;
    // int keyboard = cv::waitKey(30);
    // if (keyboard == 'q' || keyboard == 27)
    //     break;
    prvs_gray = frame_gray.clone();
      // Now update the previous frame and previous points
      cv::goodFeaturesToTrack(prvs_gray, p0, 300, 0.1, 7, cv::Mat(), 7, false, 0.04);


  }
}
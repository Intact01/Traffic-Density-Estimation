#pragma once
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

const string help_opt = "help";  // constant variables
const string video_path_opt = "file";
const string frame_rate_opt = "frameskip";
const string image_path_opt = "save";
const string choose_method_opt = "method";
const string num_threads_opt = "thread";
const string verbose_opt = "verbose";
const string resolution_opt = "resolution";

const char *help_opt_name = "help,h";
const char *video_path_opt_name = "file,f";
const char *frame_rate_opt_name = "frameskip,r";
const char *image_path_opt_name = "save,s";
const char *choose_method_opt_name = "method,m";
const char *num_threads_opt_name = "thread,t";
const char *verbose_opt_name = "verbose,v";
const char *resolution_opt_name = "resolution,x";

const string wrong_usage_msg = "Wrong usage: Use -h, --help for help";

struct Resolution {
  int width = 1920;
  int height = 1080;
};

// parses the command line args
int parse(int argc, char **argv, string &imagePath, string &videoPath,
           int &frameskip, int &method, int &num_threads,
           bool &verbose, Resolution &res) {
  string resolution;
  po::options_description desc("Allowed options");

  // allowed options' description
  desc.add_options()(help_opt_name, "produce help message")(
    video_path_opt_name, po::value<string>(&videoPath) -> default_value("input/trafficvideo.mp4"), "video path")(
    image_path_opt_name, po::value<string>(&imagePath) -> default_value("output/output.png"), "path to image of saved graph")(
    frame_rate_opt_name,po::value<int>(&frameskip) -> default_value(1), "frames to skip processing (only for method 1)")(
    choose_method_opt_name, po::value<int>(&method) -> default_value(0), "choose method to apply")(
    verbose_opt_name, "enable logging")(
    num_threads_opt_name, po::value<int>(&num_threads) -> default_value(4), "number of threads (only for methods 3,4)")(
    resolution_opt_name, po::value<string>(&resolution),"resolution for method 2 eg - 1920x1080 (only for method 2)");


  string fileName;
  try {
    // maps flags to values
    po::variables_map variable_map;
    po::store(po::parse_command_line(argc, argv, desc), variable_map);

    // show help
    if (variable_map.count(help_opt)) {
      cout << desc << endl;
      exit(0);
    }
    po::notify(variable_map);

    if (variable_map.count(verbose_opt)) {
      verbose = true;
    }
    if (variable_map.count(resolution_opt)) {
      int idx = resolution.find('x');
      if (idx == string::npos)
        cout << "wrong resolution format";
      else {
        res.height = stoi(resolution.substr(0, idx));
        res.width = stoi(resolution.substr(idx + 1));
      }
    }

    if (frameskip <= 0) {
      cout << "frameskip has to be positive" << endl;
      return 1;
    }
    if (num_threads <= 0) {
      cout << "number of threads has to be positive" << endl;
      return 1;
    }
    num_threads--;

    
    if(method != 1) frameskip = 1;
  } catch (exception &ex) {
    std::cerr << ex.what() << '\n';
    exit(0);
  }
  return 0;
}
#pragma once
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

const string help_opt = "help";  // constant variables
const string video_path_opt = "file";
const string frame_rate_opt = "frameskip";
const string image_path_opt = "save";
const string choose_section_opt = "choose";
const string choose_method_opt = "method";
const string num_threads_opt = "thread";
const string verbose_opt = "verbose";

const char *help_opt_name = "help,h";
const char *video_path_opt_name = "file,f";
const char *frame_rate_opt_name = "frameskip,r";
const char *image_path_opt_name = "save,s";
const char *choose_section_opt_name = "choose,c";
const char *choose_method_opt_name = "method,m";
const char *num_threads_opt_name = "thread,t";
const char *verbose_opt_name = "verbose,v";

const string wrong_usage_msg = "Wrong usage: Use -h, --help for help";

// parses the command line args
void parse(int argc, char **argv, string &imagePath, string &videoPath,
           int &frameskip, bool &choose, int &method, int &num_threads,
           bool &verbose) {
  // allowed options' description
  po::options_description desc("Allowed options");
  desc.add_options()(help_opt_name, "produce help message")(
      video_path_opt_name, po::value<string>(&videoPath), "video path")(
      image_path_opt_name, po::value<string>(&imagePath),"path to image of saved graph")(
      frame_rate_opt_name, po::value<int>(&frameskip),"frames to skip processing")(
      choose_section_opt_name, "enter custom area to crop")(
      choose_method_opt_name, po::value<int>(&method),"choose method to apply")(
      verbose_opt_name, "enable logging")(
      num_threads_opt_name, po::value<int>(&num_threads), "number of threads");

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

    if (variable_map.count(choose_section_opt)) {
      choose = true;
    }
    if (variable_map.count(verbose_opt)) {
      verbose = true;
    }
  } catch (const po::error &ex) {
    std::cerr << ex.what() << '\n';
    exit(0);
  }
  return;
}
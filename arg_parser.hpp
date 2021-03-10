#pragma once
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

const string help_opt = "help"; // constant variables
const string video_path_opt = "file";
const string frame_rate_opt = "framerate";
const string image_path_opt = "save";
const string choose_section_opt = "choose";

const char *help_opt_name = "help,h";
const char *video_path_opt_name = "file,f";
const char *frame_rate_opt_name = "framerate,r";
const char *image_path_opt_name = "save,s";
const char *choose_section_opt_name = "choose,c";

const string wrong_usage_msg = "Wrong usage: Use -h, --help for help";

// parses the command line args
void parse(int argc, char **argv, string &imagePath, string &videoPath, int &frameRate, bool &choose)
{
    // allowed options' description
    po::options_description desc("Allowed options");
    desc.add_options()(
        help_opt_name, "produce help message")(
        video_path_opt_name, po::value<string>(&videoPath)->required(), "video path")(
        image_path_opt_name, po::value<string>(&imagePath), "path to image of saved graph")(
        frame_rate_opt_name, po::value<int>(&frameRate), "frames to skip processing")(
        choose_section_opt_name, "enter custom area to crop");

    string fileName;
    try
    {
        // maps flags to values
        po::variables_map variable_map;
        po::store(po::parse_command_line(argc, argv, desc), variable_map);

        // show help
        if (variable_map.count(help_opt))
        {
            cout << desc << endl;
            exit(0);
        }
        po::notify(variable_map);

        if (variable_map.count(choose_section_opt))
        {
            choose = true;
        }
    }
    catch (const po::error &ex)
    {
        std::cerr << ex.what() << '\n';
        exit(0);
    }
    return;
}
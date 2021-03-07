#pragma once
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

const string help_opt = "help"; // constant variables
const string video_path_opt = "file";
const string frame_rate_opt = "framerate";
const string image_path_opt = "save";

const char *help_opt_name = "help,h";
const char *video_path_opt_name = "file,f";
const char *frame_rate_opt_name = "framerate,r";
const char *image_path_opt_name = "save,s";

const string wrong_usage_msg = "Wrong usage: Use -h, --help for help";

// class ArgumentOptions
// {
// public:
//     string videoPath, imagePath;
//     int frameRate;

//     ArgumentOptions(string videoPath, string imagePath, int frameRate);
// };

// parses the command line args
void parse(int argc, char **argv, string &imagePath, string &videoPath, int &frameRate)
{
    // allowed options' description
    po::options_description desc("Allowed options");
    desc.add_options()(
        help_opt_name, "produce help message")(
        video_path_opt_name, po::value<string>(&videoPath)->required(), "video path")(
        image_path_opt_name, po::value<string>(&imagePath), "path to image of saved graph")(
        frame_rate_opt_name, po::value<int>(&frameRate), "frame rate (fps)");

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

        // input filename
        // checks if destination points will be custom or not
        // if (variable_map.count(video_path_opt))
        // {
        //     fileName = variable_map[video_path_opt].as<string>();
        //     options.videoPath = fileName;

        //     if (variable_map.count(frame_rate_opt))
        //     {
        //         options.frameRate = variable_map[frame_rate_opt].as<int>();
        //     }
        //     if (variable_map.count(image_path_opt))
        //     {
        //         options.imagePath = variable_map[image_path_opt].as<string>()
        //     }
        // }

        // // any other argument will show wring usage message
        // else
        // {
        //     cout << wrong_usage_msg << endl;
        // }
    }
    catch (const po::error &ex)
    {
        std::cerr << ex.what() << '\n';
        exit(0);
    }
    return;
}
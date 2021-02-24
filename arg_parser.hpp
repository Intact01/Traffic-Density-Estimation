#pragma once
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

const string file_name_opt = "file";
const string help_opt = "help";
const string custom_input_opt = "custom";
const char *help_opt_name = "help,h";
const char *file_name_opt_name = "file,f";
const char *custom_input_opt_name = "custom,c";

pair<string, bool> parse(int argc, char **argv)
{
    pair<string, bool> options = make_pair("", false);

    po::options_description desc("Allowed options");
    desc.add_options()(help_opt_name, "produce help message")(file_name_opt_name, po::value<string>()->required(), "image filename")(custom_input_opt_name, "enter custom destination points");

    string fileName;
    try
    {
        po::variables_map variable_map;
        po::store(po::parse_command_line(argc, argv, desc), variable_map);

        if (variable_map.count(help_opt))
        {
            cout << desc << endl;
            return options;
        }

        po::notify(variable_map);
        if (variable_map.count(file_name_opt))
        {
            fileName = variable_map[file_name_opt].as<string>();
            options.first = fileName;

            if (variable_map.count(custom_input_opt))
            {
                options.second = true;
            }
        }
        else
        {
            cout << "Wrong usage: Use -h, --help for help" << endl;
        }
    }
    catch (const po::error &ex)
    {
        std::cerr << ex.what() << '\n';
    }
    return options;
}
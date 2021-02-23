#pragma once
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

const string file_name_opt = "filename";
const string help_opt = "help";

const char *file_name_opt_name = "filename,f";

string parse(int argc, char **argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(file_name_opt_name, po::value<string>(), "image filename");
    string fileName;
    try
    {
        po::variables_map variable_map;
        po::store(po::parse_command_line(argc, argv, desc), variable_map);
        po::notify(variable_map);
        if (variable_map.count(file_name_opt))
        {
            fileName = variable_map[file_name_opt].as<string>();
            return fileName;
        }
        else if (variable_map.count(help_opt))
        {
            cout << desc << endl;
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
    return "";
}
#include "matplotlibcpp.h"

using namespace std;
namespace plt = matplotlibcpp;

void make_graph(vector<double> queue_density_list, vector<double> moving_density_list, string &fileName, int frameRate)
{
    int n = 5000;
    vector<double> time;

    for (int i = 0; i < moving_density_list.size(); ++i)
    {
        time.push_back(i * frameRate / 15);
    }

    // Set the size of output image to 1200x780 pixels
    plt::figure_size(1200, 780);
    // Plot line from given x and y data. Color is selected automatically.
    plt::named_plot("Moving Density", time, moving_density_list);
    cout << queue_density_list.size() << " " << moving_density_list.size() << endl;

    plt::named_plot("Queue Density", time, queue_density_list);

    plt::xlim(0, (int)moving_density_list.size() * frameRate / 15);
    // Add graph title
    plt::title("Traffic Density Plot");
    // Enable legend.
    plt::legend();

    // Save the image (file format is determined by the extension)
    plt::save(fileName);
}
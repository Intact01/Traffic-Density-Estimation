#include <bits/stdc++.h>
using namespace std;

// find utility for queue density calculation
double find_utility_qd(vector<double> queue_density, int frameskip) {
  char comma = ',';

  fstream fin;
  fin.open("output/density.csv", ios::in);  // baseline file
  vector<string> row;
  int counter = 0, index = 0;
  string temp, line, word;
  getline(fin, line);
  double qd_base, qd, error_qd, total_error = 0.0, max_error;

  // read line by line from the csv file
  while (fin >> line) {
    stringstream sstream(line);

    // separate line at comma
    while (getline(sstream, word, comma)) {
      row.push_back(word);
    }

    //in case of frameskip use previous density value
    if (counter % frameskip != 0) {
      qd = queue_density[index - 1];
    } else {
      qd = queue_density[index];
      index++;
    }

    qd_base = stod(row[1]); // baseline value

    error_qd = qd - qd_base;  
    // scaling the error between 0 to 1
    max_error = max(qd_base, 1 - qd_base);  
    error_qd = error_qd / max_error;
    logger.log(to_string(counter) + " " + to_string(error_qd));

    total_error += pow(error_qd, 2);
    logger.log("Error :" + to_string(total_error));

    row.clear();
    counter++;
  }
  logger.log("Error :" + to_string(total_error) + " " + to_string(counter));

  total_error = pow((abs(total_error) / counter), 0.5); //final error
  double utility = 1 - total_error; //utility

  logger.log("Util :" + to_string(utility));
  return utility;
}

double find_utility_md(vector<double> moving_density, int frameskip) {
  char comma = ',';

  fstream fin;
  fin.open("output/density.csv", ios::in);  // baseline file
  vector<string> row;
  int counter = 0, index = 0;
  string temp, line, word;
  getline(fin, line);
  double md_base, md, error_md, total_error, max_error;

  // read line by line from the csv file
  while (fin >> line) {
    stringstream sstream(line);

    // separate line at comma
    while (getline(sstream, word, comma)) {
      row.push_back(word);
    }

    //in case of frameskip use previous density value
    if (counter % frameskip != 0) {
      md = moving_density[index - 1];
    } else {
      md = moving_density[index];
      index++;
    }

    md_base = stod(row[2]); // baseline value

    error_md = md - md_base;
    // scaling the error between 0 to 1
    max_error = max(md_base, 1 - md_base);
    error_md = error_md / max_error;
    total_error += pow(error_md, 2);
    row.clear();
    counter++;
  }
  total_error = pow((total_error / counter), 0.5);  //total error
  double utility = 1 - total_error;   // utility
  return utility;
}
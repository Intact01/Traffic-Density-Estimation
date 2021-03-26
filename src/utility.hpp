#include <bits/stdc++.h>
using namespace std;

double find_utility_qd(vector<double> queue_density, int frameskip) {
  char comma = ',';

  fstream fin;
  fin.open("output/density.csv", ios::in);
  vector<string> row;
  int counter = 0, index = 0;
  string temp, line, word;
  getline(fin, line);
  double qd_base, qd, error_qd, total_error, max_error;
  while (fin >> line) {
    counter++;
    if (counter % frameskip != 0) {
      continue;
    }
    stringstream sstream(line);
    while (getline(sstream, word, comma)) {
      // add all the column data
      // of a row to a vector
      row.push_back(word);
    }
    qd = queue_density[index];

    qd_base = stod(row[1]);

    index++;

    error_qd = qd - qd_base;
    max_error = max(qd_base, 1-qd_base);
    error_qd = error_qd/max_error;
    cout << error_qd << endl;
    
    total_error += pow(error_qd, 2);
    row.clear();
  }
  if (index != queue_density.size()) {
    cout << "gadbad kar di" << endl;
    exit(0);
  }
  total_error = pow((total_error / queue_density.size()), 0.5);
  double utility = 1 - total_error;
  return utility;
}

double find_utility_md(vector<double> moving_density, int frameskip) {
  char comma = ',';

  fstream fin;
  fin.open("output/density.csv", ios::in);
  vector<string> row;
  int counter = 0, index = 0;
  string temp, line, word;
  getline(fin, line);
  double md_base, md, error_md, total_error, max_error;
  while (fin >> line) {
    counter++;
    if (counter % frameskip != 0) {
      continue;
    }
    stringstream sstream(line);
    while (getline(sstream, word, comma)) {
      // add all the column data
      // of a row to a vector
      row.push_back(word);
    }
    md = moving_density[index];

    md_base = stod(row[2]);

    index++;

    error_md = md - md_base;
    max_error = max(md_base, 1-md_base);
    error_md = error_md/max_error;
    cout << error_md << endl;
    total_error += pow(error_md, 2);
    row.clear();
  }
  if (index != moving_density.size()) {
    cout << "gadbad kar di" << endl;
    exit(0);
  }
  total_error = pow((total_error / moving_density.size()), 0.5);
  double utility = 1 - total_error;
  return utility;
}
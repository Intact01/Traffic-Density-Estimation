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
    stringstream sstream(line);
    while (getline(sstream, word, comma)) {
      // add all the column data
      // of a row to a vector
      row.push_back(word);
    }
    if (counter % frameskip != 0) {
      qd = queue_density[index - 1];
    } else {
      qd = queue_density[index];
      index++;
    }

    qd_base = stod(row[1]);

    error_qd = qd - qd_base;
    max_error = max(qd_base, 1 - qd_base);
    error_qd = error_qd / max_error;
    // cout << error_qd << endl;

    total_error += pow(error_qd, 2);
    row.clear();
    counter++;
  }
  total_error = pow((total_error / counter), 0.5);
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
    stringstream sstream(line);
    while (getline(sstream, word, comma)) {
      // add all the column data
      // of a row to a vector
      row.push_back(word);
    }
    if (counter % frameskip != 0) {
      md = moving_density[index - 1];
    } else {
      md = moving_density[index];
      index++;
    }

    md_base = stod(row[2]);

    error_md = md - md_base;
    max_error = max(md_base, 1 - md_base);
    error_md = error_md / max_error;
    // cout << error_md << endl;
    total_error += pow(error_md, 2);
    row.clear();
    counter++;
  }
  total_error = pow((total_error / counter), 0.5);
  double utility = 1 - total_error;
  return utility;
}
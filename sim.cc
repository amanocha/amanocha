#include <unordered_map>
#include <vector>
#include <set>
#include <bitset>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <bits/stdc++.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <chrono>

#define NUM_SETS 2048
#define BLK_OFFSET 64

using namespace std;

vector<string> split(const string &s, char delim) {
  stringstream ss(s);
  string item;
  vector<string> tokens;
  while (getline(ss, item, delim)) {
    tokens.push_back(item);
  }
  return tokens;
}

template <typename T>
string join(const T& vec, char delim) {
  ostringstream new_str;
  new_str << delim;
  for (const auto& entry : vec) {
    if (&entry != &vec[0]) new_str << delim;
    new_str << entry;
  }
  return new_str.str();
}

unordered_map<uint64_t, unsigned long> global_freq; // map of all tag frequencies
unordered_map<string, unsigned long> seq; // map of all tag sequences

uint64_t extract(int max, int min, uint64_t address) { //inclusive
  uint64_t maxmask = ((uint64_t)1 << (max+1))-1;
  uint64_t minmask = ((uint64_t)1 << (min))-1;
  uint64_t mask = maxmask - minmask;
  uint64_t val = address & mask;
  val = val >> min;
  return val;
}

void simulate(ifstream& memfile) {
  string line;
  int num_lines = 0;

  if (memfile.is_open()) {
    cout << "Started reading the memory trace!\n";
    getline(memfile, line);
    vector<string> parsing;
    unsigned long ld_id, time;
    uint64_t address, pc, tag, prev_tag = 0;
    int hit;
    string tag_seq;

    while (getline(memfile, line)) {
      // Parse memory trace
      parsing = split(line, ',');
      ld_id = stoull(parsing.at(0));
      time = stoull(parsing.at(1));
      address = stoull(parsing.at(2), 0, 16);
      pc = stoull(parsing.at(3), 0, 16);
      hit = stoi(parsing.at(4));

      tag = address/NUM_SETS/BLK_OFFSET;
      if (global_freq.find(tag) == global_freq.end()) global_freq[tag] = 0;
      global_freq[tag]++; 

      tag_seq = to_string(prev_tag) + "_" + to_string(tag);
      if (seq.find(tag_seq) == seq.end()) seq[tag_seq] = 0;
      seq[tag_seq]++;
      prev_tag = tag;

      num_lines++;
    }

    if ( memfile.eof() ) {
      cout << "Finished reading the memory trace!\nLines read: " << num_lines << "\n";
      memfile.close();
    }
  }
}

int main(int argc, char** argv) {

  // PARSE ARUGMENTS
  string filename = argv[1];
  string basename = filename.substr(filename.find_last_of("/\\") + 1);
  string expname = split(basename, '.').at(0);
  ifstream memfile(filename);
  ofstream freqfile, seqfile;

  // STATS SETUP
  string dir = "output/";
  vector<string> args(argv+2, argv+argc);
  string arg_names = join(args, '_');
  dir.append(expname);
  //dir.append(arg_names);
  int status = mkdir(dir.c_str(), 0777);
  if (!status) {
    cout << "Output directory created!" << endl;
  } else {
    cout << "Unable to create output directory!" << endl;
  }

  freqfile.open(dir + "/freqs.txt");
  seqfile.open(dir + "/seqs.txt");

  // SIMULATION
  auto start = chrono::system_clock::now();
  simulate(memfile);
  auto end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  cout << "Simulation Time: " << elapsed_seconds.count() << endl;
  
  for (auto const &pair : global_freq) {
    freqfile << pair.first << " " << pair.second << endl;
  }

  for (auto const &pair : seq) {
    seqfile << pair.first << " " << pair.second << endl;
  }

  // CLEANUP
  freqfile.close();
  seqfile.close();

  return 0;
}

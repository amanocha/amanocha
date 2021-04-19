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

unordered_map<uint64_t, unsigned long> global_addr_freq; // map of all addr frequencies
unordered_map<uint64_t, unsigned long> global_tag_freq; // map of all tag frequencies
unordered_map<uint64_t, unsigned long> global_pc_freq; // map of all pcs
unordered_map<string, unsigned long> addr_seq; // map of all addr sequences
unordered_map<string, unsigned long> tag_seq; // map of all tag sequences
unordered_map<string, unsigned long> tag_pc; // map of tag+pc combinations

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
    uint64_t address, prev_address, pc, tag, prev_tag = 0;
    int hit;
    string addr_seq_str, tag_seq_str, tag_pc_str;

    while (getline(memfile, line)) {
      // Parse memory trace
      parsing = split(line, ',');
      ld_id = stoull(parsing.at(0));
      time = stoull(parsing.at(1));
      address = stoull(parsing.at(2), 0, 16);
      pc = stoull(parsing.at(3), 0, 16);
      hit = stoi(parsing.at(4));

      if (global_addr_freq.find(address) == global_addr_freq.end()) global_addr_freq[address] = 0;
      global_addr_freq[address]++;

      addr_seq_str = to_string(prev_address) + "_" + to_string(address);
      if (addr_seq.find(addr_seq_str) == addr_seq.end()) addr_seq[addr_seq_str] = 0;
      addr_seq[addr_seq_str]++;
      prev_address = address;

      tag = address/NUM_SETS/BLK_OFFSET;
      if (global_tag_freq.find(tag) == global_tag_freq.end()) global_tag_freq[tag] = 0;
      global_tag_freq[tag]++; 

      tag_seq_str = to_string(prev_tag) + "_" + to_string(tag);
      if (tag_seq.find(tag_seq_str) == tag_seq.end()) tag_seq[tag_seq_str] = 0;
      tag_seq[tag_seq_str]++;
      prev_tag = tag;

      if (global_pc_freq.find(pc) == global_pc_freq.end()) global_pc_freq[pc] = 0;
      global_pc_freq[pc]++;

      tag_pc_str = to_string(tag) + "_" + to_string(pc);
      if (tag_pc.find(tag_pc_str) == tag_pc.end()) tag_pc[tag_pc_str] = 0;
      tag_pc[tag_pc_str]++;

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
  string expname = basename.substr(0, basename.size() - 4);
  cout << basename << " " << expname << endl;
  ifstream memfile(filename);
  ofstream addrfile, tagfile, pcfile, addr_seq_file, tag_seq_file, tag_pc_file;

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

  addrfile.open(dir + "/addr.txt");
  tagfile.open(dir + "/tags.txt");
  pcfile.open(dir + "/pcs.txt");
  addr_seq_file.open(dir + "/addr_seqs.txt");
  tag_seq_file.open(dir + "/tag_seqs.txt");
  tag_pc_file.open(dir + "/tag_pc.txt");

  // SIMULATION
  auto start = chrono::system_clock::now();
  simulate(memfile);
  auto end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  cout << "Simulation Time: " << elapsed_seconds.count() << endl;
  
  for (auto const &pair : global_addr_freq) {
    addrfile << pair.first << " " << pair.second << endl;
  }

  for (auto const &pair : global_tag_freq) {
    tagfile << pair.first << " " << pair.second << endl;
  }

  for (auto const &pair : global_pc_freq) {
    pcfile << pair.first << " " << pair.second << endl;
  }

  for (auto const &pair : addr_seq) {
    addr_seq_file << pair.first << " " << pair.second << endl;
  }

  for (auto const &pair : tag_seq) {
    tag_seq_file << pair.first << " " << pair.second << endl;
  }

  for (auto const &pair : tag_pc) {
    tag_pc_file << pair.first << " " << pair.second << endl;
  }

  // CLEANUP
  addrfile.close();
  tagfile.close();
  pcfile.close();
  addr_seq_file.close();
  tag_seq_file.close();
  tag_pc_file.close();

  return 0;
}

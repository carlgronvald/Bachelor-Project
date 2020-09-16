#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <vector>
#include <sstream>
#include <utility>
#include "lodepng.h"
#include <iomanip>

std::string readFile(const char* filename);

std::vector<std::pair<std::string, std::vector<float>>> readCsv(std::string filename, char delimiter=',');

std::string padnumber(int number);
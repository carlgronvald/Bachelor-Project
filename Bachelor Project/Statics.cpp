#pragma once
#include "Statics.h"

std::string readFile(const char* filename) {
	std::string text;
	std::ifstream myfile(filename);

	myfile.seekg(0, std::ios::end);
	text.reserve(myfile.tellg());
	myfile.seekg(0, std::ios::beg);

	text.assign((std::istreambuf_iterator<char>(myfile)),
		std::istreambuf_iterator<char>());
	return text;
}

std::vector<std::pair<std::string, std::vector<float>>> readCsv(std::string filename, char delimiter) {
	// Reads a CSV file into a vector of <string, vector<int>> pairs where
	// each pair represents <column name, column values>

	// Create a vector of <string, int vector> pairs to store the result
	std::vector<std::pair<std::string, std::vector<float>>> result;

	// Create an input filestream
	std::ifstream myFile(filename);

	std::cout << filename << std::endl;

	// Make sure the file is open
	if (!myFile.is_open()) throw std::runtime_error("Could not open file");

	std::cout << "Making strings!" << std::endl;
	// Helper vars
	std::string line, colname;
	float val;
	std::cout << "Made strings! " << std::endl;

	// Read the column names
	if (myFile.good())
	{
		// Extract the first line in the file
		std::getline(myFile, line);

		// Create a stringstream from line
		std::stringstream ss(line);

		// Extract each column name
		while (std::getline(ss, colname, delimiter)) {
			std::cout << "reading something " << colname << std::endl;
			// Initialize and add <colname, int vector> pairs to result
			result.push_back({ colname, std::vector<float> {} });
		}
	}

	// Read data, line by line
	while (std::getline(myFile, line))
	{
		// Create a stringstream of the current line
		std::stringstream ss(line);

		// Keep track of the current column index
		int colIdx = 0;

		// Extract each integer
		while (ss >> val) {

			// Add the current float to the 'colIdx' column's values vector
			result.at(colIdx).second.push_back(val);

			// If the next token is a comma, ignore it and move on
			if (ss.peek() == delimiter) ss.ignore();

			// Increment the column index
			colIdx++;
		}
	}

	// Close file
	myFile.close();

	return result;
}


std::string padnumber(int number) {
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(3) << number;
	return ss.str();
}
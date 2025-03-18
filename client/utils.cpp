#include <iostream>
#include <fstream>
#include "exceptions.hpp"
#include "utils.hpp"


void printBytes(const uint8_t data[], size_t data_size) {
    for (int i =0; i < data_size; i++) {
        unsigned char byte = data[i];
        std::cout << std::hex << (int)byte << " ";
    }
    std::cout << std::hex << std::endl;
}

std::string read_file(const std::string filename) {
    std::fstream file(filename);
	if (!file || !file.is_open()) {
		throw stringable_client_exception("Couldn't open file: " + filename + " make sure the file exists and you have proper permissions to read it");
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
	return content;
}
/**
* Save the content into temp file in the temp directory, 
* Returns path to the file
*/
std::string save_file_in_temp_directory(const std::string& content, const std::string relative_path) {
	std::string filepath = "%tmp%" + relative_path;
	std::ofstream file(filepath);
    if (!file || !file.is_open()) {
		throw stringable_client_exception("Couldn't open file: " + filepath + " make sure you have proper permissions to write to the directory");
    }
	file << content;
	file.close();
	return filepath;
}

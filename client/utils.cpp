#include <iostream>  
#include <fstream>  
#include <sstream>  
#include <iomanip>  
#include "exceptions.hpp"  
#include "utils.hpp"  
#include <filesystem>


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
	std::string filepath = std::filesystem::temp_directory_path().string() + relative_path;
	std::ofstream file(filepath);  
	   if (!file || !file.is_open()) {  
		throw stringable_client_exception("Couldn't open file: " + filepath + " make sure you have proper permissions to write to the directory");  
	   }  
	file << content;  
	file.close();  
	return filepath;  
}  


std::string get_timestamp() {  
   std::ostringstream oss;  
   oss << std::time(nullptr);  
   return oss.str();  
}  

std::string bytes_to_hex_string(const uint8_t* bytes, const size_t length) {  
	std::ostringstream oss;  
	oss << std::hex << std::setfill('0');  
	for (int i = 0; i < length; i++) {  
		oss << std::setw(2) << (int)bytes[i];  
	}  
	return oss.str();  
}

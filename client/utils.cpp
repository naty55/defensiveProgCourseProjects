#include <iostream>  
#include <fstream>  
#include <sstream>  
#include <iomanip>  
#include "exceptions.hpp"  
#include "utils.hpp"  
#include <filesystem>
#include <base64.h>



std::string read_file(const std::string filename) {  
	std::fstream file(filename);  
	if (!file || !file.is_open()) {  
		throw stringable_client_exception("Couldn't open file: " + filename + " make sure the file exists and you have proper permissions to read it");  
	}  
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());  
	file.close();  
	return content;
}  

void write_file(const std::string &filename, const std::string &content) {
	std::ofstream out_file(filename);
	if (!out_file || !out_file.is_open()) {
		throw stringable_client_exception("Couldn't open file: " + filename + " make sure the file exists and you have proper permissions to write to it");
	}
	try {
		out_file.write(content.c_str(), content.size());
		out_file.close();
	}
	catch (...) {
		out_file.close();
		throw stringable_client_exception("Error when writing to file");
	}
}

/**  
* Save the content into temp file in the temp directory,   
* Returns path to the file  
*/  
std::string save_file_in_temp_directory(const std::string &content, const std::string &relative_path) {  
	std::string filepath = std::filesystem::temp_directory_path().string() + relative_path;
	std::ofstream file(filepath);  
	if (!file || !file.is_open()) {  
	throw stringable_client_exception("Couldn't open file: " + filepath + " make sure you have proper permissions to write to the directory");  
	}  
	try {
		file << content;
	}
	catch (...) {
		file.close();
		throw stringable_client_exception("Error occurred when writing to file..");
	}
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
/**
* For this function to work correctly - make sure hex size is even before passing it to the function
*/
void hex_string_to_bytes(const std::string& hex, std::vector<uint8_t> &bytes) {
	bytes.clear();
	for (size_t i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
		bytes.push_back(byte);
	}
}
/**
* Code supplied by lecurers 
*/
std::string encode_base64(const std::string& str) {
	std::string encoded;
	CryptoPP::StringSource ss(str, true,
		new CryptoPP::Base64Encoder(
			new CryptoPP::StringSink(encoded))
	);
	return encoded;
}

std::string decode_base64(const std::string& str) {
	std::string decoded;
	CryptoPP::StringSource ss(str, true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StringSink(decoded)
		)
	);

	return decoded;

}

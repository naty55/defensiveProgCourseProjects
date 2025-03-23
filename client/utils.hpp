#pragma once
#include <vector>

std::string read_file(const std::string filename);
void write_file(const std::string& filename, const std::string& content);
std::string save_file_in_temp_directory(const std::string& content, const std::string& relative_path);
std::string get_timestamp();
std::string bytes_to_hex_string(const uint8_t *bytes, const size_t length);
void hex_string_to_bytes(const std::string& hex, std::vector<uint8_t>& bytes);


std::string encode_base64(const std::string& str);
std::string decode_base64(const std::string& str);

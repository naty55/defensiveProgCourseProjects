#pragma once

std::string read_file(const std::string filename);
std::string save_file_in_temp_directory(const std::string& content, const std::string relative_path);
std::string get_timestamp();
std::string bytes_to_hex_string(const uint8_t *bytes, const size_t length);
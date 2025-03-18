#pragma once

#include <stdexcept>

class stringable_client_exception : public std::runtime_error {
public:
	stringable_client_exception(const std::string& message) : std::runtime_error(message) {}
	
	friend std::ostream& operator<<(std::ostream& os, const stringable_client_exception& e);
	
};
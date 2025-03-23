#include "exceptions.hpp"
#include <string>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const stringable_client_exception& e) {
	os << "Error: " << e.what() << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const critical_client_exception& e) {
	os << "CRITICAL ERROR: " << e.what() << "\n";
	os << "EXITING!\n";
	return os;
}

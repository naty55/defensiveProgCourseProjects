#include <iostream>
#include "utils.hpp"

void printBytes(const uint8_t data[], size_t data_size) {
    for (int i =0; i < data_size; i++) {
        unsigned char byte = data[i];
        std::cout << std::hex << (int)byte << " ";
    }
    std::cout << std::hex << std::endl;
}
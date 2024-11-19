#include <boost/asio.hpp>
#include <iostream>
#include "config.hpp"
#include "objects.hpp"
#include <array>

using boost::asio::ip::tcp;

void printBytes(char data[], int data_size) {
        for (int i =0; i < data_size; i++) {
            unsigned char byte = data[i];
            std::cout << std::hex << (int)byte << " ";
        }
        std::cout << std::hex << std::endl;
}

int main() {
    unsigned char clientId[16] = {0};
    char someString[] = "Hello";
    Request req(clientId, 1, 600, (unsigned int)6, someString);
   boost::asio::io_context io_context;
   tcp::socket s(io_context);
   tcp::resolver resolver(io_context);
   boost::system::error_code error;
   boost::asio::connect(s, resolver.resolve("localhost", "1357"));
   size_t request_size = req.size_in_bytes();
   char buffer[BUFFER_SIZE] = {};
   req.to_bytes(buffer, BUFFER_SIZE);
   std::cout << "Req bytes: ";
   printBytes(buffer, 29);
   boost::asio::write(s, boost::asio::buffer(buffer, request_size), error);
   if (error) {
    std::cout << error.message() << std::endl;
   }
   std::array<char, BUFFER_SIZE> response_buffer;
   size_t len = s.read_some(boost::asio::buffer(response_buffer), error);
   if( error && error != boost::asio::error::eof ) {
        std::cout << "receive failed: " << error.message() << std::endl;
    }
    else {
        char* data = response_buffer.data();
        std::cout << "Response: ";
        std::cout.write(data, len);
        // printBytes(response_buffer.data(), 28);
        Response res(data, len);
        std::cout << "Parsed res: " << res << std::endl;
    }
   return 0;
}
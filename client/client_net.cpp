#include <boost/asio.hpp>
#include <iostream>
#include "config.hpp"
#include "objects.hpp"
#include <memory>

using boost::asio::ip::tcp;

static void printBytes_(const uint8_t data[], int data_size) {
    for (int i =0; i < data_size; i++) {
        uint8_t byte = data[i];
        std::cout << std::hex << (int)byte << " ";
    }
    std::cout << std::hex << std::endl;
}


std::unique_ptr<Response> send_request(Request &request) {
    boost::asio::io_context ctx;
    tcp::resolver resolver(ctx);
    auto endpoint = resolver.resolve(SERVER_HOST, SERVER_PORT);
    tcp::socket s(ctx);
    boost::system::error_code error;
    boost::asio::connect(s, endpoint);
    size_t request_size = request.size_in_bytes();
    char buffer[BUFFER_SIZE] = {0};
    request.to_bytes(buffer, BUFFER_SIZE);
    boost::asio::write(s, boost::asio::buffer(buffer, request_size), error);
    if (error) {
        std::cout << error.message() << std::endl;
    }
    std::array<uint8_t, BUFFER_SIZE> response_buffer;
    size_t len = s.read_some(boost::asio::buffer(response_buffer), error);
    if( error && error != boost::asio::error::eof ) {
            std::cout << "receive failed: " << error.message() << std::endl;
    }
    else {
        const uint8_t* data = response_buffer.data();
        std::cout << "response bytes: ";
        printBytes_(data, len);
        s.close();
        return std::make_unique<Response>(data, len);
    }
    return nullptr;
}
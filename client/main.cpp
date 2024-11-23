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
std::unique_ptr<Response> send_request(Request &request);
void client_loop();
void handle_command(std::string &line, char client_id[HEADER_CLIENT_ID_SIZE]);

int main() {
    client_loop();
    return 0;
}

void client_loop(){
    bool running = true;
    char client_id[HEADER_CLIENT_ID_SIZE] = {0};
    while (running) {
        std::cout << CLIENT_MESSAGE << std::endl;
        std::string line; 
        std::getline(std::cin, line);
        std::cout<< line << "\n";
        try {
            handle_command(line, client_id);
        } catch (const std::exception& e) {
            std::cout << "Caught a standard exception: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Caught an unknown exception." << std::endl;
        }
        
    }
}

void handle_command(std::string &line, char client_id[HEADER_CLIENT_ID_SIZE]) {
    int input_num = 0;
    try {
        input_num = std::stoi(line);
    } catch(...) {
        std::cout << "It's not you it's me - please select again" << std::endl;
    }
    std::unique_ptr<Response> res;

    switch (input_num) {
        case 110: {
            client_id[HEADER_CLIENT_ID_SIZE] = {0};
            unsigned char client_name[255] = "Hello";
            unsigned char client_public_id[160] = "1111111111111111111111111111";
            unsigned char payload[255 + 160] = {0};
            std::memcpy(payload, client_name, 255);
            std::memcpy(payload + 255, client_public_id, 160);
            Request req(client_id, 1, 600, (unsigned long int) (255 + 160), payload);
            std::cout << "Request: " << req << std::endl;
            res = send_request(req);
            char * res_payload = res.get()->getPayload();
            std::memcpy(client_id, res_payload, HEADER_CLIENT_ID_SIZE);
            std::cout << "Registered! client_id=";
            printBytes(client_id, HEADER_CLIENT_ID_SIZE);
            std::cout << "client_name=" << client_name << std::endl;
            break;
        } 
        case 120: {
            Request req(client_id, 1, 601, (unsigned long int) 0, nullptr);
            std::cout << "Request: " << req << std::endl;
            res = send_request(req);
            char * res_payload = res.get()->getPayload();
            int user_count = res.get()->getPayloadSize() / (16 + 255);
            if (res.get()->getPayloadSize() % (16 + 255)) {
                std::cout << "Wrong size of payload" << std::endl;
                raise;
            }
            std::cout << "Users count: " << user_count << std::endl; 
            std::cout << "Users:" << std::endl;

            for (int i=0; i < user_count; i++) {
                char user_name[255] ={0}; 
                char user_id[16] = {0};
                std::memcpy(user_id, res_payload + i * (16 + 255), 16);
                std::memcpy(user_name, res_payload + i * (16 + 255) + 16, 255);
                std::cout << "User: " << user_name << std::endl;
            }
            
            break;

        } 
        case 130: {
            Request req(client_id, 1, 602, (unsigned long int) 0, nullptr);
            res = send_request(req);
            break;
        }
        case 140: {
            break;
        }
        case 150: {

        }
        case 151: {

        }
        case 152: {

        }
        case 0: {
            exit(0);
        }
        default: {
            break;
        }
    }
    std::cout << "Response: " <<  *res.get() << std::endl;
    std::cout << "Done" << std::endl;

}

std::unique_ptr<Response> send_request(Request &request) {
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    auto endpoint = resolver.resolve(SERVER_HOST, SERVER_PORT);
    tcp::socket s(io_context);
    boost::system::error_code error;
    boost::asio::connect(s, endpoint);
    size_t request_size = request.size_in_bytes();
    char buffer[BUFFER_SIZE] = {0};
    request.to_bytes(buffer, BUFFER_SIZE);
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
        s.close();
        return std::make_unique<Response>(data, len);
    }
    return nullptr;
}
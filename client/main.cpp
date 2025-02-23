#include <boost/asio.hpp>
#include <iostream>
#include "config.hpp"
#include "objects.hpp"
#include "protocol.hpp"
#include "client.hpp"
#include <array>
#include <string>
#include <memory>
#include "client_net.hpp"

void printBytes(unsigned char data[], int data_size) {
        for (int i =0; i < data_size; i++) {
            unsigned char byte = data[i];
            std::cout << std::hex << (int)byte << " ";
        }
        std::cout << std::hex << std::endl;
}
std::unique_ptr<Response> send_request(Request &request);
void client_loop();
bool handle_command(std::string &line, Client& client);

int main() {
    client_loop();
    std::cout << "See you later!" << std::endl;
    return 0;
}

void client_loop(){
    bool running = true;
    Client client;
    while (running) {
        std::cout << CLIENT_MESSAGE << std::endl;
        std::string line; 
        std::getline(std::cin, line);
        std::cout<< line << "\n";
        try {
            running = handle_command(line, client);
        } catch (const std::exception& e) {
            std::cout << "Caught a standard exception: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Caught an unknown exception." << std::endl;
        }
    }
}

bool handle_command(std::string &line, Client& client) {
    int input_num = 0;
    try {
        input_num = std::stoi(line);
    } catch(...) {
        std::cout << "It's not you it's me - please select again" << std::endl;
        return true;
    }

    if (0 == input_num) {
        std::cout << "Exiting" << std::endl;
        return false;
    }

    if (!client.isRegistered() && input_num != 110) {
        std::cout << "You need to register first" << std::endl;
        return true;
    }
    
    std::unique_ptr<Response> res;

    switch (input_num) {
        case 110: {
            std::string client_name;
            while (client_name.size() + 1 > HEADER_CLIENT_NAME_SIZE || client_name.size() == 0) {
                std::cout << "Please enter your name: ";
                std::cin >> client_name;
            }
            unsigned char client_public_key[HEADER_CLIENT_PUBLIC_KEY_SIZE] = "1111111111111111111111111111";
            unsigned char payload[HEADER_CLIENT_NAME_SIZE + HEADER_CLIENT_PUBLIC_KEY_SIZE] = {0};
            std::memcpy(payload, client_name.c_str(), client_name.size());
            std::memcpy(payload + HEADER_CLIENT_NAME_SIZE, client_public_key, HEADER_CLIENT_PUBLIC_KEY_SIZE);
            Request req(client.getClientId(), 1, RequestCode::REQ_REGISTRATION, (unsigned long int) (HEADER_CLIENT_NAME_SIZE + HEADER_CLIENT_PUBLIC_KEY_SIZE), payload);
            
            std::cout << "Request: " << req << std::endl;
            res = send_request(req);
            std::cout << "Response: " << *res << std::endl;

            if (res.get()->getResponseCode() != ResponseCode::RES_REGISTRATION) {
                std::cout << "Failed to register" << std::endl;
                break;
            }
            uint8_t * res_payload = res.get()->getPayload();
            uint8_t client_id[HEADER_CLIENT_ID_SIZE] = {0};
            std::memcpy(client_id, res_payload, HEADER_CLIENT_ID_SIZE);
            client.setClientId(client_id);
            client.setClientName(client_name);
            client.setRegistered(true);
            std::cout << "Registered! client_id=";
            printBytes(client_id, HEADER_CLIENT_ID_SIZE);
            std::cout << "client_name=" << client_name << std::endl;
            break;
        } 
        case 120: {
            Request req(client.getClientId(), 1, RequestCode::REQ_CLIENTS_LIST, (unsigned long int) 0, nullptr);
            std::cout << "Request: " << req << std::endl;
            res = send_request(req);
            uint8_t * res_payload = res.get()->getPayload();
            int user_count = res.get()->getPayloadSize() / (16 + 255);
            if (res.get()->getPayloadSize() % (16 + 255)) {
                std::cout << "Wrong size of payload" << std::endl;
                raise;
            }
            std::cout << "Users count: " << user_count << std::endl; 
            std::cout << "Users:" << std::endl;

            for (int i=0; i < user_count; i++) {
                char user_name[HEADER_CLIENT_NAME_SIZE] ={0}; 
                char user_id[HEADER_CLIENT_ID_SIZE] = {0};
                std::memcpy(user_id, res_payload + i * (16 + 255), 16);
                std::memcpy(user_name, res_payload + i * (16 + 255) + 16, 255);
                std::cout << "User: " << user_name << std::endl;
            }
            
            break;

        } 
        case 130: {
            uint8_t target_client_id[HEADER_CLIENT_ID_SIZE] = "12345";
            Request req(client.getClientId(), 1, RequestCode::REQ_PUBLIC_KEY, (unsigned long int) HEADER_CLIENT_ID_SIZE, target_client_id);
            res = send_request(req);
            break; 
        }
        case 140: {
            Request req(client.getClientId(), 1, RequestCode::REQ_PENDING_MSGS, (unsigned long int) 0, nullptr);
            res = send_request(req);
            uint8_t * res_payload = res.get()->getPayload();
            printBytes(res_payload, res.get()->getPayloadSize());
            break; 
        }
        case 150: {
            uint8_t to_client_id[HEADER_CLIENT_ID_SIZE] = {0};
            Message message(to_client_id, MessageType::MSG_TEXT, 12, "Hello World");
            uint8_t buffer[message.size_in_bytes()] = {0};
            message.to_bytes(buffer, message.size_in_bytes());
            Request req(client.getClientId(), 1, RequestCode::REQ_SEND_MSG, (unsigned long int) message.size_in_bytes(), buffer);
            res = send_request(req);
            uint8_t * res_payload = res.get()->getPayload();
            printBytes(res_payload, res.get()->getPayloadSize());
            break;
        }
        case 151: {
            unsigned char to_client_id[HEADER_CLIENT_ID_SIZE] = {0};
            Message message(to_client_id, MessageType::MSG_SYMMETRIC_KEY_REQUEST, 0, nullptr);
            uint8_t buffer[message.size_in_bytes()] = {0};
            message.to_bytes(buffer, message.size_in_bytes());
            Request req(client.getClientId(), 1, RequestCode::REQ_SEND_MSG, (unsigned long int) message.size_in_bytes(), buffer);
            res = send_request(req);
            uint8_t* res_payload = res.get()->getPayload();
            printBytes(res_payload, res.get()->getPayloadSize());
            break;

        }
        case 152: {
            uint8_t to_client_id[HEADER_CLIENT_ID_SIZE] = {0};
            Message message(to_client_id, MessageType::MSG_SYMMETRIC_KEY_SEND, 0, nullptr);
            uint8_t buffer[message.size_in_bytes()] = {0};
            message.to_bytes(buffer, message.size_in_bytes());
            Request req(client.getClientId(), 1, RequestCode::REQ_SEND_MSG, (unsigned long int) message.size_in_bytes(), buffer);
            res = send_request(req);
            uint8_t * res_payload = res.get()->getPayload();
            printBytes(res_payload, res.get()->getPayloadSize());
            break;
        }
        case 0: {
            exit(0);
        }
        default: {
            break;
        }
    }
    return true;
}

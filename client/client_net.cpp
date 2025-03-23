#include <boost/asio.hpp>
#include <iostream>
#include "config.hpp"
#include "objects.hpp"
#include "protocol.hpp"
#include "exceptions.hpp"
#include "utils.hpp"
#include <memory>

using boost::asio::ip::tcp;

static void validate_response_code(const uint8_t* first_packet, ResponseCode expected) {
    const ResponseHeader* header = reinterpret_cast<const ResponseHeader*>(first_packet);
    ResponseCode response_code = static_cast<ResponseCode>(header->response_code);
    if (response_code == ResponseCode::RES_SERVER_ERROR) {
        throw stringable_client_exception("Server returned with error");
    }
    if (response_code != expected) {
        throw stringable_client_exception("Unexpected response code for request - couldn't complete the request");
    }
}

static size_t validate_size(const uint8_t* first_packet, const size_t read_length, const bool expect_variable_payload_length) {
    if (read_length < sizeof(ResponseHeader)) {
        throw stringable_client_exception("Network error - response is too small might be because of slow network");
    }
    const ResponseHeader* header = reinterpret_cast<const ResponseHeader*>(first_packet);
    size_t expected_size = header->payload_size + sizeof(ResponseHeader);
    if ((read_length != expected_size && !expect_variable_payload_length) || (expect_variable_payload_length && expected_size < read_length)) {
        throw stringable_client_exception("Network error - Wrong size of response");
    }
    return expected_size;
}

static void send_request(tcp::socket &s, boost::asio::io_context &ctx, const Request& request, const std::string &SERVER_HOST, const std::string &SERVER_PORT) {
    boost::system::error_code error;
    tcp::resolver resolver(ctx);
    auto endpoint = resolver.resolve(SERVER_HOST, SERVER_PORT);
    boost::asio::connect(s, endpoint);
    std::vector<char> buffer(request.size_in_bytes());
    request.to_bytes(buffer);
    boost::asio::write(s, boost::asio::buffer(buffer, request.size_in_bytes()), error);
    if (error) {
        throw stringable_client_exception("Network error - sending request failed");
    }
}

static size_t recieve_response(tcp::socket& s, std::vector<uint8_t> &response_buffer, ResponseCode expected_response_code, const bool expect_variable_payload_length) {
    response_buffer.clear();
    boost::system::error_code error;
    std::array<uint8_t, BUFFER_SIZE> temp;
    size_t read_length = s.read_some(boost::asio::buffer(temp, BUFFER_SIZE), error);
    if (error && error != boost::asio::error::eof) {
        throw stringable_client_exception("Network error - receive data failed");
    }
    response_buffer.insert(response_buffer.end(), temp.begin(), temp.begin() + read_length);
    const size_t expected_size = validate_size(temp.data(), read_length, expect_variable_payload_length);
    validate_response_code(temp.data(), expected_response_code);

    while (expect_variable_payload_length && read_length < expected_size) {
        size_t temp_length = s.read_some(boost::asio::buffer(temp, BUFFER_SIZE), error);
        response_buffer.insert(response_buffer.end(), temp.begin(), temp.begin() + temp_length);
        read_length += temp_length;
    }
    if (read_length != expected_size) {
        throw stringable_client_exception("Network error resposne is corrupted");
    }
    return read_length;
}

std::unique_ptr<Response> send_request_and_get_response(Request &request, ResponseCode expected_response_code, const std::string &SERVER_HOST, const std::string &SERVER_PORT) {
	RequestCode request_code = request.getRequestCode();
	bool expect_variable_payload_length = request_code == RequestCode::REQ_CLIENTS_LIST || request_code == RequestCode::REQ_PENDING_MSGS;
    boost::asio::io_context ctx;
    tcp::socket s(ctx);
    try {
        send_request(s, ctx, request, SERVER_HOST, SERVER_PORT);
    }
    catch (stringable_client_exception& e) {
            throw e;
    } 
    catch (...) {
        throw stringable_client_exception("Network error occurred while sending request");
    }


    std::vector<uint8_t> response_buffer;
    try {
        size_t read_length = recieve_response(s, response_buffer, expected_response_code, expect_variable_payload_length);
        s.close();
        return std::make_unique<Response>(response_buffer.data(), read_length);
    }
    catch (stringable_client_exception& e) {
        throw e;
    }
    catch (...) {
        throw stringable_client_exception("Network error occurred while reading response");
    }
}
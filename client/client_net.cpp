#include <boost/asio.hpp>
#include <iostream>
#include "config.hpp"
#include "objects.hpp"
#include "protocol.hpp"
#include "exceptions.hpp"
#include <memory>

using boost::asio::ip::tcp;

std::unique_ptr<Response> send_request(Request &request, ResponseCode expectedResponseCode) {
	RequestCode request_code = request.getRequestCode();
	bool expect_variable_payload_length = request_code == RequestCode::REQ_CLIENTS_LIST || request_code == RequestCode::REQ_PENDING_MSGS;
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
        throw stringable_client_exception("Network error - sending request failed");
    }
    std::array<uint8_t, BUFFER_SIZE> response_buffer;
    size_t len = s.read_some(boost::asio::buffer(response_buffer), error);
    if( error && error != boost::asio::error::eof ) {
        throw stringable_client_exception("Network error - receive data failed");
    }
    const uint8_t* first_packet = response_buffer.data();
	const ResponseHeader* header = reinterpret_cast<const ResponseHeader*>(first_packet);
	size_t expected_size = header->payload_size + sizeof(ResponseHeader);
    ResponseCode response_code = static_cast<ResponseCode>(header->response_code);
    if (response_code == ResponseCode::RES_SERVER_ERROR) {
        throw stringable_client_exception("Server returned with error");
    }
    if (response_code != expectedResponseCode) {
        throw stringable_client_exception("Unexpected response code for request - couldn't complete the request");
    }
	if ((len != expected_size && !expect_variable_payload_length) || (expect_variable_payload_length && expected_size < len)) {
		throw stringable_client_exception("Network error - Wrong size of response");
	}
	if (expect_variable_payload_length && len < expected_size) { 
		
	}
    s.close();
    return std::make_unique<Response>(first_packet, len);
}
#include <boost/asio.hpp>
#include <iostream>
#include "config.hpp"
#include "objects.hpp"
#include "protocol.hpp"
#include "exceptions.hpp"
#include <memory>

using boost::asio::ip::tcp;

static void validate_response_code(const ResponseHeader *header, ResponseCode expected) {
    ResponseCode response_code = static_cast<ResponseCode>(header->response_code);
    if (response_code == ResponseCode::RES_SERVER_ERROR) {
        throw stringable_client_exception("Server returned with error");
    }
    if (response_code != expected) {
        throw stringable_client_exception("Unexpected response code for request - couldn't complete the request");
    }
}

static size_t validate_size(const ResponseHeader* header, const size_t read_length, const bool expect_variable_payload_length) {
    size_t expected_size = header->payload_size + sizeof(ResponseHeader);
    if ((read_length != expected_size && !expect_variable_payload_length) || (expect_variable_payload_length && expected_size < read_length)) {
        throw stringable_client_exception("Network error - Wrong size of response");
    }
    return expected_size;
}

static void send_request(tcp::socket &s, boost::asio::io_context &ctx, const Request& request) {
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

std::unique_ptr<Response> send_request_and_get_response(Request &request, ResponseCode expected_response_code) {
	RequestCode request_code = request.getRequestCode();
	bool expect_variable_payload_length = request_code == RequestCode::REQ_CLIENTS_LIST || request_code == RequestCode::REQ_PENDING_MSGS;
    boost::system::error_code error;
    boost::asio::io_context ctx;
    tcp::socket s(ctx);
    send_request(s, ctx, request);
    
    std::array<uint8_t, BUFFER_SIZE> response_buffer;
    size_t read_length = s.read_some(boost::asio::buffer(response_buffer), error);
    if( error && error != boost::asio::error::eof ) {
        throw stringable_client_exception("Network error - receive data failed");
    }
    const uint8_t* first_packet = response_buffer.data();
    const ResponseHeader* header = reinterpret_cast<const ResponseHeader*>(first_packet);
	
    validate_response_code(header, expected_response_code);
    const size_t expected_size = validate_size(header, read_length, expect_variable_payload_length);

    const uint8_t* data = first_packet;
    std::vector<uint8_t> resposne_vector(read_length);
	while (expect_variable_payload_length && read_length < expected_size) { 
        

        
		
	}
    s.close();
    return std::make_unique<Response>(data, read_length);
}
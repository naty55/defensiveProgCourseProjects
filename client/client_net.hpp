#pragma once
#include "objects.hpp"
#include <memory>

std::unique_ptr<Response> send_request_and_get_response(Request &request, ResponseCode expected_response_code);
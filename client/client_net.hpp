#pragma once
#include "objects.hpp"
#include <memory>

std::unique_ptr<Response> send_request(Request &request, ResponseCode expectedResponseCode);
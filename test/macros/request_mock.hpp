#pragma once

#include "request/Request.hpp"
#include "shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <string>
#include <unordered_map>

struct RequestRecording {
    RequestRecording(
        const std::string &verb, const std::string &url,
        const std::unordered_map<
            CloudSync::request::Request::ParameterType, const std::unordered_map<std::string, std::string>> &parameters,
        const std::string &body)
        : verb(verb), url(url), parameters(parameters), body(body){};
    const std::string verb;
    const std::string url;
    const std::unordered_map<
        CloudSync::request::Request::ParameterType, const std::unordered_map<std::string, std::string>>
        parameters;
    const std::string body;
};

static std::vector<RequestRecording> requestRecording;

#define INIT_REQUEST()                                                                                                 \
    requestRecording.clear();                                                                                          \
    SHARED_PTR_MOCK(request, request::Request);

#define WHEN_REQUEST() When(Method((requestMock), request))

#define RESPOND(returnvalue)                                                                                           \
    Do([](const std::string &verb,                                                                                     \
          const std::string &url,                                                                                      \
          const std::unordered_map<                                                                                    \
              CloudSync::request::Request::ParameterType,                                                              \
              const std::unordered_map<std::string, std::string>>                                                      \
              parameters,                                                                                              \
          const std::string &body) {                                                                                   \
        requestRecording.push_back(RequestRecording(verb, url, parameters, body));                                     \
        return returnvalue;                                                                                            \
    })

#define REQUIRE_REQUEST(number, condition) REQUIRE(requestRecording.at(number).condition)

#define REQUIRE_REQUEST_CALLED() Verify(Method((requestMock), request))

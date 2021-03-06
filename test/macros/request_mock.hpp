#pragma once

#include "request/Request.hpp"
#include "shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

struct RequestRecording {
    RequestRecording(
        std::string verb,
        std::string url)
        : verb(std::move(verb))
        , url(std::move(url)) {};
    const std::string verb;
    const std::string url;
    std::string body;
    std::vector<std::uint8_t> binary_body;
    std::string bearer_token;
    std::string basic_username;
    std::string basic_password;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query_params;
    std::unordered_map<std::string, std::string> postfields;
    std::unordered_map<std::string, std::string> mime_postfields;
};

static std::vector<RequestRecording> requestRecording;

#define INIT_REQUEST()                                                                                                 \
    requestRecording.clear();                                                                                          \
    SHARED_PTR_MOCK(request, request::Request);                                                                        \
    When(Method(requestMock, resource)).AlwaysDo([request](const std::string& verb, const std::string& url){           \
        requestRecording.emplace_back(verb, url);                                                                      \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, header)).AlwaysDo([request](const std::string& key, const std::string& value){            \
        requestRecording.back().headers.insert({key, value});                                                          \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, query_param)).AlwaysDo([request](const std::string& key, const std::string& value){       \
        requestRecording.back().query_params.insert({key, value});                                                     \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, postfield)).AlwaysDo([request](const std::string& key, const std::string& value){         \
        requestRecording.back().postfields.insert({key, value});                                                       \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, mime_postfield)).AlwaysDo([request](const std::string& key, const std::string& value){    \
        requestRecording.back().mime_postfields.insert({key, value});                                                  \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, basic_auth)).AlwaysDo([request](const std::string& username, const std::string& password){\
        requestRecording.back().basic_username = username;                                                             \
        requestRecording.back().basic_password = password;                                                             \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, token_auth)).AlwaysDo([request](const std::string& token){                                \
        requestRecording.back().bearer_token = token;                                                                  \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, body)).AlwaysDo([request](const std::string& content){                                    \
        requestRecording.back().body = content;                                                                        \
        return request;                                                                                                \
    });                                                                                                                \
    When(Method(requestMock, binary_body)).AlwaysDo([request](const std::vector<std::uint8_t>& content){               \
        requestRecording.back().binary_body = content;                                                                 \
        return request;                                                                                                \
    })

#define REQUIRE_REQUEST(number, condition) REQUIRE(requestRecording.at(number).condition)

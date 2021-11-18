#include "CurlRequest.hpp"
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std::chrono;
using namespace std::literals::chrono_literals;

namespace CloudSync::request::curl {
    CurlRequest::CurlRequest() {
        this->curl = curl_easy_init();
    }

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        ((std::string *) userp)->append((char *) contents, size * nmemb);
        return size * nmemb;
    }

    static size_t HeaderCallback(char *contents, size_t size, size_t nmemb, void *userp) {
        const std::string header = std::string((char *) contents, size * nmemb);
        const auto separatorPosition = header.find_first_of(':');
        if (separatorPosition != std::string::npos) {
            std::string key = header.substr(0, separatorPosition);
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            const std::string value = header.substr(separatorPosition + 1);
            // trim away unwanted leading and trailing whitespaces
            size_t start = value.find_first_not_of(CurlRequest::WHITESPACE);
            size_t end = value.find_last_not_of(CurlRequest::WHITESPACE);
            const std::string trimmedValue = value.substr(start, (end - start) + 1);
            ((std::unordered_map<std::string, std::string> *) userp)->insert({key, trimmedValue});
        }
        return size * nmemb;
    }

    const std::string CurlRequest::WHITESPACE = " \n\r";

    Response CurlRequest::request(
            const std::string &verb, const std::string &url,
            const std::unordered_map<ParameterType, const std::unordered_map<std::string, std::string>> &parameters,
            const std::string &body) {

        if (!this->tokenRequestUrl.empty() && (!this->accessToken.empty() || !this->refreshToken.empty())) {
            this->refreshOAuth2TokenIfNeeded();
        }

        struct curl_slist *headers = nullptr;
        curl_mime *form = nullptr;
        // enable redirects
        if (this->optionFollowRedirects)
            curl_easy_setopt(this->curl, CURLOPT_FOLLOWLOCATION, 1);

        // set verbose
        if (this->optionVerbose)
            curl_easy_setopt(this->curl, CURLOPT_VERBOSE, 1);

        // authorization
        if (!this->username.empty() && !this->password.empty()) {
            // Basic Auth
            curl_easy_setopt(this->curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            curl_easy_setopt(this->curl, CURLOPT_USERNAME, this->username.c_str());
            curl_easy_setopt(this->curl, CURLOPT_PASSWORD, this->password.c_str());
        } else if (!this->tokenRequestUrl.empty() && (!this->accessToken.empty() || !this->refreshToken.empty())) {
            // OAuth2
            headers = curl_slist_append(headers, std::string("Authorization: Bearer " + this->accessToken).c_str());
        }

        // apply proxy
        if (!this->proxyUrl.empty()) {
            curl_easy_setopt(this->curl, CURLOPT_PROXY, this->proxyUrl.c_str());
            if (!this->proxyUser.empty() && !this->proxyPassword.empty()) {
                const auto escapedProxyUser =
                        curl_easy_escape(this->curl, this->proxyUser.c_str(), this->proxyUser.length());
                const auto escapedProxyPassword =
                        curl_easy_escape(this->curl, this->proxyPassword.c_str(), this->proxyPassword.length());
                const auto proxyUserPwd = std::string(escapedProxyUser) + ":" + std::string(escapedProxyPassword);
                curl_free(escapedProxyUser);
                curl_free(escapedProxyPassword);
                curl_easy_setopt(this->curl, CURLOPT_PROXYUSERPWD, proxyUserPwd.c_str());
            }
        }

        // append query params to url
        std::string finalUrl = url;
        const auto queryParams = parameters.find(QUERY_PARAMS);
        if (queryParams != parameters.end()) {
            if (!queryParams->second.empty())
                finalUrl += "?";
            finalUrl += CurlRequest::urlEncodeParams(queryParams->second);
        }

        // set url (including potential query params
        curl_easy_setopt(this->curl, CURLOPT_URL, finalUrl.c_str());

        // set headers
        const auto headerParams = parameters.find(HEADERS);
        if (headerParams != parameters.end()) {
            for (const auto &header: headerParams->second) {
                headers = curl_slist_append(headers, (header.first + ": " + header.second).c_str());
            }
        }
        curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, headers);

        if (verb != "GET") {
            if (verb == "HEAD") {
                curl_easy_setopt(this->curl, CURLOPT_NOBODY, 1);
            } else {
                curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, verb.c_str());
            }
            if (!body.empty()) {
                curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, body.c_str());
            } else {
                const auto postfields = parameters.find(POSTFIELDS);
                const auto mimePostfields = parameters.find(MIME_POSTFIELDS);
                const auto mimePostfiles = parameters.find(MIME_POSTFILES);
                if (postfields != parameters.end()) {
                    curl_easy_setopt(
                            this->curl,
                            CURLOPT_POSTFIELDS,
                            CurlRequest::urlEncodeParams(postfields->second).c_str());
                } else if (mimePostfields != parameters.end() || mimePostfiles != parameters.end()) {
                    form = curl_mime_init(this->curl);

                    if (mimePostfields != parameters.end()) {
                        for (const auto &field: mimePostfields->second) {
                            curl_mimepart *mimePart = curl_mime_addpart(form);
                            curl_mime_name(mimePart, field.first.c_str());
                            curl_mime_data(mimePart, field.second.c_str(), CURL_ZERO_TERMINATED);
                        }
                    }
                    if (mimePostfiles != parameters.end()) {
                        for (const auto &file: mimePostfiles->second) {
                            curl_mimepart *mimePart = curl_mime_addpart(form);
                            curl_mime_filename(mimePart, "upload");
                            curl_mime_name(mimePart, file.first.c_str());
                            curl_mime_data(mimePart, file.second.c_str(), CURL_ZERO_TERMINATED);
                        }
                    }
                    curl_easy_setopt(this->curl, CURLOPT_MIMEPOST, form);
                }
            }
        }
        // perform request
        std::string responseReadBuffer;
        std::unordered_map<std::string, std::string> responseHeaders;
        long responseCode;
        char *responseContentType;
        char errorbuffer[CURL_ERROR_SIZE];
        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &responseReadBuffer);
        curl_easy_setopt(this->curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(this->curl, CURLOPT_HEADERDATA, &responseHeaders);
        curl_easy_setopt(this->curl, CURLOPT_ERRORBUFFER, errorbuffer);
        const auto requestResult = curl_easy_perform(this->curl);
        const auto responseCodeInfoResult = curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &responseCode);
        const auto dataTypeInfoResult = curl_easy_getinfo(this->curl, CURLINFO_CONTENT_TYPE, &responseContentType);

        // cleanup after request
        if (form != nullptr)
            curl_mime_free(form);
        curl_slist_free_all(headers);

        // return result
        if (requestResult == CURLE_OK && responseCodeInfoResult == CURLE_OK && dataTypeInfoResult == CURLE_OK) {
            // when the response has no body, responseContentType is a nullptr. This needs to be checked when
            // transforming the char* to a string.
            const std::string responseContentTypeString = responseContentType ? std::string(responseContentType) : "";
            auto response = Response(responseCode, responseReadBuffer, responseContentTypeString, responseHeaders);
            curl_easy_reset(this->curl);
            return response;
        } else {
            const auto errorMessage = std::string(errorbuffer);
            curl_easy_reset(this->curl);
            throw RequestException(errorMessage);
        }
    }

    std::string CurlRequest::urlEncodeParams(const std::unordered_map<std::string, std::string> &params) const {
        std::string result;
        bool firstLoopIteration = true;
        for (const auto &param: params) {
            if (firstLoopIteration)
                firstLoopIteration = false;
            else
                result += "&";
            const auto key = curl_easy_escape(this->curl, param.first.c_str(), param.first.size());
            const auto value = curl_easy_escape(this->curl, param.second.c_str(), param.second.size());
            result += std::string(key) + "=" + std::string(value);
            curl_free(key);
            curl_free(value);
        }
        return result;
    }

    CurlRequest::~CurlRequest() {
        curl_easy_cleanup(this->curl);
    }
} // namespace CloudSync::request::curl

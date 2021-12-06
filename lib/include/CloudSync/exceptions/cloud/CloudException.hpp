#pragma once

#include "CloudSync/exceptions/Exception.hpp"

namespace CloudSync::exceptions::cloud {
    /// Base-exception for all cloud-related errors.
    class CloudException : public Exception {
    public:
        explicit CloudException(const std::string &what) : Exception(what) {};
    };

    /**
     * @brief Thrown if authorization to the cloud has failed.
     *
     * @warning Be aware that this may be thrown at any time, not just on the `login()` call. For example the password
     * could be changed or the OAuth2 token could be revoked at any point in time. Recovering from this is only possible
     * by asking the user for new credentials.
     */
    class AuthorizationFailed : public CloudException {
    public:
        AuthorizationFailed() : CloudException("Login to the cloud failed.") {};
    };

    /**
     * @brief Thrown if the communication to with the server failed.
     *
     * In an ideal world this would never be thrown. Catching this is an indicator that you found a bug in the library
     * or, less likely, in the providers API. It's most likely not possible to recover from this.
     *
     * Possible reasons why this occurs:
     *     * A request has failed before it even reached the server. This may be due to a wrong usage of the request
     *       library or, less likely, a problem with the request library itself (libCURL).
     *     * The server responded with an unexpected result. Unexpected means that the library does not know how to
     *       handle it because the implementation or the response does not follow the API-spec.
     */
    class CommunicationError : public CloudException {
    public:
        explicit CommunicationError(const std::string &what = "")
                : CloudException("The communication with the server has failed. " + what) {};
    };

    /**
     @brief Thrown if the server response cannot be parsed.
     *
     * This is a special case of a CommunicationError. When using the library in production catching
     * CommunicationErrors should be enough.
     * This exists to make debugging easier.
     *
     * It's not possible to recover from this.
     *
     */
    class InvalidResponse : public CommunicationError {
    public:
        explicit InvalidResponse(const std::string &what = "") : CommunicationError("Invalid Response: " + what) {};
    };
}
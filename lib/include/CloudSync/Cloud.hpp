#pragma once

#include "Credentials.hpp"
#include "Directory.hpp"
#include "Exceptions.hpp"
#include "Proxy.hpp"
#include <string>

namespace CloudSync {

class Cloud {
  public:
    // MARK: - errors

    /**
     * @brief Thrown if a provided interface/functionality is not implemented by the used cloud provider.
     *
     * If this error is being thrown it means the API has been used in a wrong way.
     */
    class MethodNotSupportedError : public std::logic_error {
      public:
        explicit MethodNotSupportedError(const std::string &what = "")
            : std::logic_error("This method is not supported by this cloud provider. " + what){};
    };

    /// Base Exception for all Cloud-related errors.
    class CloudException : public BaseException {
      public:
        explicit CloudException(const std::string &what) : BaseException(what){};
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
        AuthorizationFailed() : CloudException("Login to the cloud failed."){};
    };

    /**
     * @brief Thrown if the communication to with the server failed.
     *
     * In an ideal world this would never be thrown. Catching this is an indicator that you found a bug in the library
     * or, less likely, in the providers API. It's most likely not possible to recover from this.
     *
     * Possible reasons why this occurs:
     *     * A request has failed before it even reached the server. This may be due to a wrong usage of the request
     * library or, less likely, a problem with the request library itself (libCURL).
     *     * The server responded with an unexpected result. Unexpected means that the library does not know how to
     * handle it because the implementation or the response does not follow the API-spec.
     */
    class CommunicationError : public CloudException {
      public:
        explicit CommunicationError(const std::string &what = "")
            : CloudException("The communication with the server has failed. " + what){};
    };

    /**
     @brief Thrown if the server response cannot be parsed.
     *
     * This is a special case of a CommunicationError. When using the library in production catching CommunicationErrors
     should be enough.
     * This exists to make debugging easier.
     *
     * It's not possible to recover from this.
     *
     */
    class InvalidResponse : public CommunicationError {
      public:
        explicit InvalidResponse(const std::string &what = "") : CommunicationError("Invalid Response: " + what){};
    };

    // checks if the cloud storage is available
    // MARK: - interface
    /**
     * Use this to check if the connection to the cloud is working. This currently is just a shorthand for
     * `cloud->root()->ls();`.
     * @throws AuthorizationFailed if your login-credentials are wrong.
     */
    virtual void ping() const = 0;
    /**
     * set login credentials
     * @param credentials either UsernamePasswordCredentials or OAuth2Credentials
     */
    virtual std::shared_ptr<Cloud> login(const Credentials &credentials) = 0;

    /**
     * @warning This is only functional if **login** was done with OAuth2Credentials and the providers OAuth2
     * implementation uses a refresh-token. Otherwise it will just return an empty string.
     * @return the current refresh token, as the OAuth2 spec allows it to change over time. Use this to save your
     * current refresh token right before the cloud instance gets destroyed. You can then use it to start a new session
     * at a later time.
     */
    virtual std::string getCurrentRefreshToken() const = 0;

    virtual std::shared_ptr<Cloud> proxy(const Proxy &proxy) = 0;
    /**
     * @throws MethodNotSupportedError if no authorize-URL can be provided.
     * @return the URL that can be used to authorize at the cloud. Not supported by all providers:
     *          * **WebDav**: Not supported
     *          * **Nextcloud**: URL to Nextclouds
     * [Login-Flow](https://docs.nextcloud.com/server/18/developer_manual/client_apis/LoginFlow/index.html) endpoint.
     *          * **Dropbox, Box, Onedrive**: Url to the OAuth2 `/authorize` endpoint
     */
    virtual std::string getAuthorizeUrl() const = 0;
    /**
     * @return the URL to the OAuth2 `/token` endpoint if the cloud supports OAuth2-Login. Otherwise just returns an
     * empty string.
     */
    virtual std::string getTokenUrl() const = 0;

    virtual std::string getBaseUrl() const = 0;

    /**
     * Fetches the users display name (!= username, usually its first- & lastname)
     * @warning this makes a network call every time it is called.
     * @return the users display name.
     */
    virtual std::string getUserDisplayName() const = 0;
    /**
     * @return root directory. This is the entrypoint for all file operations.
     * @code
     * auto file = cloud->root()->file("path/to/file.txt");
     * @endcode
     */
    virtual std::shared_ptr<Directory> root() const = 0;
};

} // namespace CloudSync

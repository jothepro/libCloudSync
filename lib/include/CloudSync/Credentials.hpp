#pragma once

#include <memory>

namespace CloudSync {
    namespace request {
        class Request;
    }
    class CloudImpl;

    class Credentials {
        friend class CloudImpl;

    protected:
        /**
         * @exception CloudSync::Credentials::InvalidCredentials
         * @exception CloudSync::Cloud::CommunicationError
         */
        virtual void apply(const std::shared_ptr<request::Request> &request) const {};
    };

} // namespace CloudSync

#pragma once

#include "CloudSync/Cloud.hpp"
#include "request/Request.hpp"

namespace CloudSync {
    class CloudImpl : public Cloud, public std::enable_shared_from_this<CloudImpl> {
    public:
        void test_connection() const override;

        std::string get_base_url() const override;

        virtual ~CloudImpl() = default;

    protected:
        CloudImpl(std::string url, std::shared_ptr<request::Request> request);

        std::shared_ptr<request::Request> m_request;
        std::string m_base_url;
    };
} // namespace CloudSync

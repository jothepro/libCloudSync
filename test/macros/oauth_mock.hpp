#pragma once

#include "credentials/OAuth2CredentialsImpl.hpp"
#include "shared_ptr_mock.hpp"
#include <catch2/catch.hpp>

#define OAUTH_MOCK(token)                                                                                              \
    SHARED_PTR_MOCK(credentials, CloudSync::credentials::OAuth2CredentialsImpl);                                       \
    When(Method(credentialsMock, get_current_access_token)).AlwaysReturn(token)

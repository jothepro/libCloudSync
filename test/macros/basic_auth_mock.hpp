#pragma once

#include "shared_ptr_mock.hpp"
#include <catch2/catch.hpp>
#include "credentials/BasicCredentialsImpl.hpp"

#define BASIC_AUTH_MOCK(usernamevalue, passwordvalue)                                                                  \
    SHARED_PTR_MOCK(credentials, CloudSync::credentials::BasicCredentialsImpl);                                        \
    When(Method(credentialsMock, username)).AlwaysReturn(usernamevalue);                                               \
    When(Method(credentialsMock, password)).AlwaysReturn(passwordvalue)

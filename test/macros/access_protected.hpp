#pragma once

#define ACCESS_PROTECTED(A, M)                                                                                         \
    struct M##_struct : get_a1<void A>::type {                                                                         \
        using get_a1<void A>::type::M;                                                                                 \
    }
#define CALL_PROTECTED(B, M) ((B).*&M##_struct::M)

template <typename T> struct get_a1;
template <typename R, typename A1> struct get_a1<R(A1)> { typedef A1 type; };

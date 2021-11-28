#pragma once

#define TEST(description, command) \
    std::cout << description;             \
    command;                              \
    std::cout << " ✅" << std::endl;

#define TEST_IF(description, command, condition) \
    std::cout << description;             \
    command;                              \
    if(condition) {                       \
       std::cout << " ✅" << std::endl;                                   \
    } else {                              \
         std::cout << " ❌" << std::endl;      \
         throw std::runtime_error(description); \
    }

#define TEST_THROWS(description, method, exception) \
    {                                                   \
        std::cout << description;                          \
        bool hasThrown = false;   \
        try {                                 \
            method;                                      \
        } catch (const exception &e) {        \
            hasThrown = true;      \
        }                   \
        if(hasThrown) {                                    \
            std::cout << " ✅" << std::endl;            \
        } else {                                           \
            std::cout << " ❌" << std::endl;         \
            throw std::runtime_error(description); \
        }  \
    }


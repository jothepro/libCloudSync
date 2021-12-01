#pragma once

#define SHARED_PTR_MOCK(name, type)                             \
    Mock<type> name##Mock;                                      \
    Fake(Dtor(name##Mock));                                     \
    auto name = std::shared_ptr<type>(&name##Mock());

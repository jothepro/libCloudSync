#pragma once

#define SHARED_PTR_MOCK(name, type)                                                                                    \
    const auto name##Mock = new Mock<type>();                                                                          \
    Fake(Dtor((*name##Mock)));                                                                                         \
    const auto name = std::shared_ptr<type>(&name##Mock->get());

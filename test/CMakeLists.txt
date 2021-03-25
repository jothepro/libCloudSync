cmake_minimum_required(VERSION 3.15)

project(MyLibraryTest)

find_package(Catch2 MODULE REQUIRED)

add_executable(MyLibraryTest
    mylibrarytest.cpp
)

target_link_libraries(MyLibraryTest
    Catch2::Catch2
    MyLibrary::MyLibrary
)

add_test(MyLibraryTest MyLibraryTest)

target_compile_features(MyLibraryTest PRIVATE cxx_std_17)
cmake_minimum_required(VERSION 3.4...3.22)

project(device)

find_package(PythonExtensions REQUIRED)

add_library(device MODULE device/device.cpp)
python_extension_module(device)
install(TARGETS device LIBRARY DESTINATION device)

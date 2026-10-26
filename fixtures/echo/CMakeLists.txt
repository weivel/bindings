cmake_minimum_required(VERSION 3.15)
project(echo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(echo SHARED echo.c)

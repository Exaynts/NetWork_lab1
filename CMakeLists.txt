cmake_minimum_required(VERSION 3.5)

project(lab1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаем отдельные сборки для задания каждого уровня
add_executable(Task0_Train Task0_Train.cpp)
add_executable(Task1_Basic Task1_Basic.cpp)
add_executable(Task2_Medium Task2_Medium.cpp)
add_executable(Task3_Advanced Task3_Advanced.cpp)

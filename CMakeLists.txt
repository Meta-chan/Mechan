cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(MechanTelegramInterface VERSION 1.0 LANGUAGES CXX)

find_package(Td 1.6.10 REQUIRED)

add_library(mechan_telegram_interface source/mechan_telegram_interface.cpp)
target_link_libraries(mechan_telegram_interface PRIVATE Td::TdStatic)
set_property(TARGET mechan_telegram_interface PROPERTY CXX_STANDARD 14)
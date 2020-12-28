cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(TdExample VERSION 1.0 LANGUAGES CXX)

find_package(Td 1.7.0 REQUIRED)

add_executable(mechan_telegram_interface source/mechan_telegram_interface.cpp source/mechan_socket.cpp)
target_include_directories(mechan_telegram_interface PRIVATE ~/_ir)
target_link_libraries(mechan_telegram_interface PRIVATE Td::TdStatic)
set_property(TARGET mechan_telegram_interface PROPERTY CXX_STANDARD 14)

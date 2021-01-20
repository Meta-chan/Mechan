cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(Mechan LANGUAGES CXX)
find_package(Td 1.7.0)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

if(Td_FOUND)
	add_executable(mechan_telegram_interface
	source/mechan_telegram_interface.cpp
	source/mechan_parse.cpp
	source/mechan_character.cpp
	source/mechan_socket.cpp
	source/mechan_config.cpp)
	target_include_directories(mechan_telegram_interface PRIVATE ${Ir_DIR})
	target_link_libraries(mechan_telegram_interface PRIVATE Td::TdStatic)
	set_property(TARGET mechan_telegram_interface PROPERTY CXX_STANDARD 14)
endif()

add_executable(mechan_console_interface
source/mechan_character.cpp
source/mechan_config.cpp
source/mechan_console_interface.cpp
source/mechan_parse.cpp
source/mechan_socket.cpp)
target_include_directories(mechan_console_interface PRIVATE ${Ir_DIR})

add_executable(mechan_dialog_downloader
source/mechan_character.cpp
source/mechan_dialog_downloader.cpp)
target_include_directories(mechan_dialog_downloader PRIVATE ${Ir_DIR})

add_executable(mechan
source/mechan.cpp
source/mechan_character.cpp
source/mechan_command.cpp
source/mechan_config.cpp
source/mechan_core.cpp
source/mechan_dialog.cpp
source/mechan_ir_implement.cpp
source/mechan_neuro.cpp
source/mechan_parse.cpp
source/mechan_socket.cpp
source/mechan_word.cpp)
target_include_directories(mechan PRIVATE ${Ir_DIR})
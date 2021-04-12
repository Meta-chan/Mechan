project(Mechan LANGUAGES CXX)

set(Td_DIR E:/Project/Mechan/td)
set(Vcpkg_DIR E:/Project/_lib/vcpkg)
set(Ir_DIR E:/Project/ir)
set(GLEW_DIR E:/Project/_lib/glew)
set(MathG_DIR E:/Project/MathG)
set(NeuroG_DIR E:/Project/NeuroG)

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

add_executable(mechan
source/mechan.cpp
source/mechan_character.cpp
source/mechan_config.cpp
source/mechan_core.cpp
source/mechan_dialog.cpp
source/mechan_ironic.cpp
source/mechan_neuro.cpp
source/mechan_parse.cpp
source/mechan_socket.cpp
source/mechan_word.cpp)
target_include_directories(mechan PRIVATE ${Ir_DIR}/include)
target_include_directories(mechan PRIVATE ${GLEW_DIR}/include)
target_include_directories(mechan PRIVATE ${MathG_DIR}/include)
target_include_directories(mechan PRIVATE ${NeuroG_DIR}/include)
target_link_libraries(mechan PRIVATE opengl32.lib)
target_link_libraries(mechan PRIVATE ${GLEW_DIR}/lib/Release/x64/glew32.lib)

add_executable(mechan_trainer
source/mechan_character.cpp
source/mechan_config.cpp
source/mechan_dialog.cpp
source/mechan_ironic.cpp
source/mechan_neuro.cpp
source/mechan_parse.cpp
source/mechan_trainer.cpp
source/mechan_word.cpp)
target_include_directories(mechan_trainer PRIVATE ${Ir_DIR}/include)
target_include_directories(mechan_trainer PRIVATE ${GLEW_DIR}/include)
target_include_directories(mechan_trainer PRIVATE ${MathG_DIR}/include)
target_include_directories(mechan_trainer PRIVATE ${NeuroG_DIR}/include)
target_link_libraries(mechan_trainer PRIVATE opengl32.lib)
target_link_libraries(mechan_trainer PRIVATE ${GLEW_DIR}/lib/Release/x64/glew32.lib)

add_executable(mechan_dialog_downloader
source/mechan_character.cpp
source/mechan_dialog_downloader.cpp
source/mechan_ironic.cpp)
target_include_directories(mechan_dialog_downloader PRIVATE ${Ir_DIR}/include)

add_executable(mechan_console_interface
source/mechan_character.cpp
source/mechan_config.cpp
source/mechan_console_interface.cpp
source/mechan_ironic.cpp
source/mechan_parse.cpp
source/mechan_socket.cpp)
target_include_directories(mechan_console_interface PRIVATE ${Ir_DIR}/include)

find_package(Td 1.7.0)
if(Td_FOUND)
	set(CMAKE_TOOLCHAIN_FILE ${Vcpkg_DIR})
	add_executable(mechan_telegram_interface
	source/mechan_character.cpp
	source/mechan_config.cpp
	source/mechan_ironic.cpp
	source/mechan_parse.cpp
	source/mechan_socket.cpp
	source/mechan_telegram_interface.cpp)
	target_include_directories(mechan_telegram_interface PRIVATE ${Ir_DIR}/include)
	target_link_libraries(mechan_telegram_interface PRIVATE Td::TdStatic)
	if(UNIX)
		target_link_libraries(mechan_telegram_interface PRIVATE -latomic)
	endif()
	set_property(TARGET mechan_telegram_interface PROPERTY CXX_STANDARD 14)
endif()
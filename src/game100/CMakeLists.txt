cmake_minimum_required (VERSION 3.6)

project(game100)

file(GLOB HEADER_FILES ./src/*.h)
file(GLOB SOURCE_FILES ./src/*.cpp)

add_definitions(-DUNICODE -D_UNICODE )
add_compile_options(-std:c++latest)

set(CMAKE_MFC_FLAG 2)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")


add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
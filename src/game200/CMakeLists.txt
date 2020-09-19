cmake_minimum_required (VERSION 3.6)

project(game200)

file(GLOB HEADER_FILES ./src/*.h)
file(GLOB SOURCE_FILES ./src/*.cpp)

add_definitions(-DUNICODE -D_UNICODE -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING -DPROJECT_NAME="${PROJECT_NAME}" /std:c++latest /utf-8)

set(CMAKE_MFC_FLAG 2)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")


add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}  d3d11 dxguid)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")
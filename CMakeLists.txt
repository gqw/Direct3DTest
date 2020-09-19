cmake_minimum_required (VERSION 3.6)

project(Direct3DTest)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/) 
set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.18362.0")

add_subdirectory(third/DirectXTex)

add_subdirectory(src/game100)
add_subdirectory(src/game200)
add_subdirectory(src/game300)
add_subdirectory(src/game400)
add_subdirectory(src/game500)
add_subdirectory(src/game501)
add_subdirectory(src/game600)
add_subdirectory(src/game601)
add_subdirectory(src/game602)
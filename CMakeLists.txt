cmake_minimum_required (VERSION 3.6)

project(Direct3DTest)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/) 

add_subdirectory(third/DirectXTex)

add_subdirectory(src/game100)
add_subdirectory(src/game200)
add_subdirectory(src/game300)
add_subdirectory(src/game400)
add_subdirectory(src/game500)
add_subdirectory(src/game501)
add_subdirectory(src/game600)
add_subdirectory(src/game700)
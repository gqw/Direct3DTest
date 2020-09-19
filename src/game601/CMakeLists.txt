cmake_minimum_required (VERSION 3.6)

project(game601)

file(GLOB HEADER_FILES 
	./src/*.h 
	./src/*.cpp
)
file(GLOB VIEWS_SOURCE_FILES 
	./src/Views/*.h 
	./src/Views/*.cpp
)
file(GLOB IMGUI_SOURCE_FILES 
	${CMAKE_HOME_DIRECTORY}/third/imgui/*.h 
	${CMAKE_HOME_DIRECTORY}/third/imgui/*.cpp
	${CMAKE_HOME_DIRECTORY}/third/imgui/examples/imgui_impl_win32.h
	${CMAKE_HOME_DIRECTORY}/third/imgui/examples/imgui_impl_win32.cpp
	${CMAKE_HOME_DIRECTORY}/third/imgui/examples/imgui_impl_dx11.h
	${CMAKE_HOME_DIRECTORY}/third/imgui/examples/imgui_impl_dx11.cpp
)

source_group("Imgui" FILES ${IMGUI_SOURCE_FILES}) 
source_group("Views" FILES ${VIEWS_SOURCE_FILES}) 

add_definitions(-DUNICODE -D_UNICODE -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING -DPROJECT_NAME="${PROJECT_NAME}" /std:c++latest /utf-8)

set(CMAKE_MFC_FLAG 2)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")


add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${IMGUI_SOURCE_FILES} ${VIEWS_SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_HOME_DIRECTORY}/third/imgui ${CMAKE_HOME_DIRECTORY}/third/imgui/examples)
target_link_libraries(${PROJECT_NAME}  d3d11 dxguid d3dcompiler DirectXTex)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")
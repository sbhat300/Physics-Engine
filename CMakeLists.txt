cmake_minimum_required(VERSION 3.5.0)
project(Collisions VERSION 2.5.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

configure_file(${PROJECT_SOURCE_DIR}/config.h.in config.h)

file(GLOB imgui_SRC ${PROJECT_SOURCE_DIR}/include/imgui/*.cpp)

add_executable(${PROJECT_NAME}  ${PROJECT_SOURCE_DIR}/src/collisionResolution.cpp 
                                ${PROJECT_SOURCE_DIR}/src/glad.c
                                ${PROJECT_SOURCE_DIR}/include/camera/camera2D.cpp
                                ${PROJECT_SOURCE_DIR}/include/entity.cpp
                                ${PROJECT_SOURCE_DIR}/include/fileLoader/fileLoader.cpp
                                ${PROJECT_SOURCE_DIR}/include/ImguiImplementation/imguiInitialize.cpp
                                ${PROJECT_SOURCE_DIR}/include/FileLoader/objDataLoader.cpp
                                ${PROJECT_SOURCE_DIR}/include/Objects/point.cpp
                                ${PROJECT_SOURCE_DIR}/include/Objects/polygon.cpp
                                ${PROJECT_SOURCE_DIR}/include/Physics/polygonCollider.cpp
                                ${PROJECT_SOURCE_DIR}/include/Physics/ray.cpp
                                ${PROJECT_SOURCE_DIR}/include/Shader/shader.cpp
                                ${PROJECT_SOURCE_DIR}/include/sharedData.cpp
                                ${PROJECT_SOURCE_DIR}/include/Physics/spatialHashGrid.cpp
                                ${PROJECT_SOURCE_DIR}/include/Physics/polygonRigidbody.cpp
                                ${imgui_SRC}
                                )

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/imgui)


if(WIN32)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/libglfw3dll.a)
elseif(APPLE)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/libglfw.3.dylib)
    else()
        target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/libglfw.3.4.dylib)
    endif()
endif()
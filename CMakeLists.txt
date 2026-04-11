cmake_minimum_required(VERSION 3.10.0)
project(Collisions VERSION 2.5.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

configure_file(${PROJECT_SOURCE_DIR}/config.h.in config.h)

file(GLOB imgui_SRC ${PROJECT_SOURCE_DIR}/include/imgui/*.cpp)

set(Collisions_SOURCES
    ${PROJECT_SOURCE_DIR}/src/glad.c
    ${PROJECT_SOURCE_DIR}/include/camera/camera2D.cpp
    ${PROJECT_SOURCE_DIR}/include/entity.cpp
    ${PROJECT_SOURCE_DIR}/include/FileLoader/fileLoader.cpp
    ${PROJECT_SOURCE_DIR}/include/ImguiImplementation/imguiInitialize.cpp
    ${PROJECT_SOURCE_DIR}/include/FileLoader/objDataLoader.cpp
    ${PROJECT_SOURCE_DIR}/include/Objects/point.cpp
    ${PROJECT_SOURCE_DIR}/include/Objects/polygon.cpp
    ${PROJECT_SOURCE_DIR}/include/Physics/polygonCollider.cpp
    ${PROJECT_SOURCE_DIR}/include/Physics/ray.cpp
    ${PROJECT_SOURCE_DIR}/include/Shader/shader.cpp
    ${PROJECT_SOURCE_DIR}/include/Engine/sharedData.cpp
    ${PROJECT_SOURCE_DIR}/include/Engine/inputHandler.cpp
    ${PROJECT_SOURCE_DIR}/include/Physics/spatialHashGrid.cpp
    ${PROJECT_SOURCE_DIR}/include/Physics/polygonRigidbody.cpp
    ${PROJECT_SOURCE_DIR}/include/Physics/collisionSolver.cpp
    ${PROJECT_SOURCE_DIR}/include/mathFuncs.cpp
    ${PROJECT_SOURCE_DIR}/include/Sound/audioPlayer.cpp
    ${PROJECT_SOURCE_DIR}/include/Sound/audioClip.cpp
    ${PROJECT_SOURCE_DIR}/include/Sound/audioLinkedList.cpp
    ${PROJECT_SOURCE_DIR}/include/Sound/audioManager.cpp
    ${PROJECT_SOURCE_DIR}/include/FileLoader/wavFile.cpp
    ${imgui_SRC}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/collisionResolution.cpp ${Collisions_SOURCES})
add_library(${PROJECT_NAME}_lib STATIC ${Collisions_SOURCES})
set_target_properties(${PROJECT_NAME}_lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

foreach(target ${PROJECT_NAME}_lib ${PROJECT_NAME})

    target_compile_definitions(${target} PRIVATE GLM_ENABLE_EXPERIMENTAL)

    target_include_directories(${target} PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/imgui)


    if(WIN32)
        target_link_libraries(${target} ${CMAKE_SOURCE_DIR}/lib/libglfw3dll.a ${CMAKE_SOURCE_DIR}/lib/libsoundio.dll)
    elseif(APPLE)
        if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
            target_link_libraries(${target} ${CMAKE_SOURCE_DIR}/lib/libglfw.3.dylib)
        else()
            target_link_libraries(${target} ${CMAKE_SOURCE_DIR}/lib/libglfw.3.4.dylib)
            target_link_libraries(${target} ${CMAKE_SOURCE_DIR}/lib/libsoundio.2.0.0.dylib)
        endif()
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(${target} pulse ${CMAKE_SOURCE_DIR}/lib/libglfw3.a ${CMAKE_SOURCE_DIR}/lib/libsoundio.a GL X11 pthread rt m dl)
    endif()
endforeach()
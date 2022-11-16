project(bitmap)

cmake_minimum_required(VERSION 2.8)

include_directories( ${PROJECT_SOURCE_DIR} )

add_executable(${PROJECT_NAME}
    main.cpp
    ${PROJECT_SOURCE_DIR}/window.h
    ${PROJECT_SOURCE_DIR}/window.cpp
    ${PROJECT_SOURCE_DIR}/bitmap.h
    ${PROJECT_SOURCE_DIR}/bitmap.cpp
    )

add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/aloe_bee.bmp ${CMAKE_CURRENT_BINARY_DIR}/aloe_bee.bmp)
add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/text.bmp ${CMAKE_CURRENT_BINARY_DIR}/text.bmp)

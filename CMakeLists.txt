cmake_minimum_required (VERSION 3.5.1)

project(assimp2bams)

# "For this is how God loved the world:
# he gave his only Son, so that everyone
# who believes in him may not perish
# but may have eternal life."
#
# John 3:16

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

ENABLE_LANGUAGE(CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# reset global variables
unset (CMAKE_MODULE_PATH CACHE)
unset (CMAKE_PREFIX_PATH CACHE)

add_subdirectory(libs/InteractiveToolkit "${CMAKE_BINARY_DIR}/lib/InteractiveToolkit")
add_subdirectory(libs/InteractiveToolkit-Extension "${CMAKE_BINARY_DIR}/lib/InteractiveToolkit-Extension")

# show info about the lib parameters
find_package(InteractiveToolkit REQUIRED)

include(libs/InteractiveToolkit-Extension/cmake/libassimp.cmake)

FILE( GLOB HEADERS src/*.h )
FILE( GLOB SRC src/*.cpp )

add_executable(${PROJECT_NAME}
    ${HEADERS}
    ${SRC}
)

# assimp uses c++11, so the child projects needs to use it too
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)

# set the target's folder (for IDEs that support it, e.g. Visual Studio)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "TOOLS")

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${assimp_INCLUDE_DIRS}
)

# Linking libraries
target_link_libraries( 
    ${PROJECT_NAME}
    PUBLIC
    InteractiveToolkit-Extension
    assimp
)

if(WIN32)
    tool_copy_file_after_build(
        ${PROJECT_NAME} 
        resources/assimp2bams-no-animations.bat
        resources/assimp2bams-only-animations.bat
    )
endif()


if( NOT MSVC )
    # Install setup
    install(
        TARGETS ${PROJECT_NAME} 
        RUNTIME DESTINATION bin
    )
endif()

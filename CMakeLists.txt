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
# assimp uses c++11 in their source
# set (CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake-modules)

#the path we need to get the custom lib versions
set(LIBS_REPOSITORY_URL "https://github.com/A-Ribeiro/public_libs/raw/main")

# avoid base libs overwrite
option(ARIBEIRO_SKIP_INSTALL_CORE ON)
option(ARIBEIRO_SKIP_INSTALL_PLATFORM ON)
option(ARIBEIRO_SKIP_INSTALL_DATA ON)

include(cmake/include.cmake)
include(cmake/libs/libaRibeiroCore.cmake)
include(cmake/libs/libaRibeiroPlatform.cmake)
include(cmake/libs/libaRibeiroData.cmake)
include(cmake/libs/libassimp.cmake)

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

# Linking libraries
target_link_libraries( 
    ${PROJECT_NAME}
    assimp
    aRibeiroCore aRibeiroPlatform aRibeiroData
)

if(WIN32)
    copy_file_after_build(
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

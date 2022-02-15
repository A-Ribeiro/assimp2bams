set( LIB_ASSIMP TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_ASSIMP PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromSource)

if(LIB_ASSIMP STREQUAL TryFindPackageFirst)
    #find_package(assimp QUIET)
    find_path(
        assimp_INCLUDE_DIRS
        NAMES assimp/postprocess.h assimp/scene.h assimp/version.h assimp/config.h assimp/cimport.h
        PATHS /usr/local/include
        PATHS /usr/include/
    )

    find_library(
        assimp_LIBRARIES
        NAMES assimp
        PATHS /usr/local/lib/
        PATHS /usr/lib64/
        PATHS /usr/lib/
    )

    if (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
	  set(assimp_FOUND TRUE)
	endif()

    if (assimp_FOUND)
        message(STATUS "[LIB_ASSIMP] using system lib.")
        set(LIB_ASSIMP UsingFindPackage)
    else()
        message(STATUS "[LIB_ASSIMP] compiling from source.")
        set(LIB_ASSIMP FromSource)
    endif()
endif()

if(LIB_ASSIMP STREQUAL FromSource)

    # if (NOT LIBS_REPOSITORY_URL)
    #     message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    # endif()
    # tool_download_lib_package(${LIBS_REPOSITORY_URL} freetype)
    # tool_include_lib(freetype)
    # include_directories("${ARIBEIRO_GEN_INCLUDE_DIR}/freetype/" PARENT_SCOPE)

    message(FATAL_ERROR "Compiling from Source not implemented")

elseif(LIB_ASSIMP STREQUAL UsingFindPackage)

    tool_is_lib(assimp assimp_registered)
    if (NOT ${assimp_registered})
        
        #find_package(assimp REQUIRED QUIET)

        # find_path(
        #     assimp_INCLUDE_DIRS
        #     NAMES assimp/postprocess.h assimp/scene.h assimp/version.h assimp/config.h assimp/cimport.h
        #     PATHS /usr/local/include
        #     PATHS /usr/include/
        # )

        # find_library(
        #     assimp_LIBRARIES
        #     NAMES assimp
        #     PATHS /usr/local/lib/
        #     PATHS /usr/lib64/
        #     PATHS /usr/lib/
        # )

        add_library(assimp OBJECT ${assimp_LIBRARIES})
        target_link_libraries(assimp ${assimp_LIBRARIES})
        include_directories(${assimp_INCLUDE_DIRS} PARENT_SCOPE)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(assimp PROPERTIES FOLDER "LIBS")

        tool_register_lib(assimp)

    endif()

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()

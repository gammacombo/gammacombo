# require a certain cmake version
CMAKE_MINIMUM_REQUIRED( VERSION 3.4 )

# declare the project name
PROJECT(gammacomboCore)

# set project version
SET( ${PROJECT_NAME}_VERSION_MAJOR 1 )
SET( ${PROJECT_NAME}_VERSION_MINOR 0 )
SET( ${PROJECT_NAME}_VERSION_PATCH 0 )

# install destinations can be passed via the command line:
# cmake -DLIBRARY_OUTPUT_DIRECTORY:PATH=.... -DINCLUDE_OUTPUT_DIRECTORY:PATH=....
# otherwise, set default install destinations
IF(NOT DEFINED LIBRARY_OUTPUT_DIRECTORY)
    set( LIBRARY_OUTPUT_DIRECTORY ./lib )
ENDIF()
IF(NOT DEFINED INCLUDE_OUTPUT_DIRECTORY)
    set( INCLUDE_OUTPUT_DIRECTORY ./include )
ENDIF()

# include cmake macros and settings
LIST( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )
INCLUDE(settings)
INCLUDE(macros)
INCLUDE(combiners)

# use ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# find FreeType
find_package(Freetype REQUIRED)
MESSAGE("FREETYPE_LIBRARIES: " ${FREETYPE_LIBRARIES})
#MESSAGE("FREETYPE_LIBRARY_DIR: " ${FREETYPE_LIBRARY})

# find ROOT
FIND_PACKAGE(ROOT REQUIRED)
# defines:
# ROOT_FOUND          If the ROOT is found
# ROOT_INCLUDE_DIR    PATH to the include directory
# ROOT_LIBRARIES      Most common libraries
# ROOT_GUI_LIBRARIES  Most common gui libraries
# ROOT_LIBRARY_DIR    PATH to the library directory
MESSAGE("ROOT_INCLUDE_DIR: " ${ROOT_INCLUDE_DIR})
MESSAGE("ROOT_LIBRARIES: " ${ROOT_LIBRARIES})
MESSAGE("ROOT_GUI_LIBRARIES: " ${ROOT_GUI_LIBRARIES})
MESSAGE("ROOT_LIBRARY_DIR: " ${ROOT_LIBRARY_DIR})

# find Boost
SET(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Enable fix for FindBoost.cmake")
FIND_PACKAGE(Boost REQUIRED)
IF (Boost_FOUND)
    MESSAGE(STATUS "Boost is found at: ${Boost_INCLUDE_DIR}")
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()

# find Doxygen
FIND_PACKAGE(Doxygen)

if(DOXYGEN_FOUND)
    # add combiner modules to Doxygen
    set(COMBINER_MODULES_DOXYGEN_SOURCES)
    foreach( dir ${COMBINER_MODULES} )
        set(COMBINER_MODULES_DOXYGEN_SOURCES "${COMBINER_MODULES_DOXYGEN_SOURCES}
            INPUT += ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include
            INPUT += ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/src"
        )
    endforeach()
    # configure Doxygen config file
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)
    # add target
    ADD_CUSTOM_TARGET(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
ELSE()
    MESSAGE( STATUS "Doxygen not installed or found -- API documentation can't be generated!" )
ENDIF(DOXYGEN_FOUND)

######################################
#
# check for C++ version
#
######################################

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O3")
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++11 HAS_CPP11)
check_cxx_compiler_flag(-std=c++0x HAS_CPP11_ALTERNATIVE)
check_cxx_compiler_flag(-std=c++14 HAS_CPP14)
check_cxx_compiler_flag(-std=c++1y HAS_CPP14_ALTERNATIVE)
check_cxx_compiler_flag(-std=c++17 HAS_CPP17)
check_cxx_compiler_flag(-std=c++1z HAS_CPP17_ALTERNATIVE)

if (HAS_CPP17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif (HAS_CPP17_ALTERNATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
elseif (HAS_CPP14)
    message(WARNING "No C++17 support.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif (HAS_CPP14_ALTERNATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
else()
    message(WARNING "No C++14 support.")
    if (HAS_CPP11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif (HAS_CPP11_ALTERNATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        message(FATAL_ERROR "No C++11 support.")
    endif()
endif()

######################################
#
# set includes
#
######################################

# Set core include directory
SET( CORE_INC_DIR ./core/include/ )

# include directories
INCLUDE_DIRECTORIES(
    BEFORE
    ${CORE_INC_DIR}
    ${ROOT_INCLUDE_DIR}
)
# declare the ROOT library path
LINK_DIRECTORIES(${ROOT_LIBRARY_DIR} /usr/local/lib)

# define the sources of the core library
AUX_SOURCE_DIRECTORY( ./core/src CORE_LIB_SOURCES )

######################################
#
# generate ROOT dictionaries
#
######################################

SET(CORE_DICTIONARY_SOURCES
    RooBinned2DBicubicBase.h
    RooCrossCorPdf.h
    #RooHistInterpol.h
    RooHistPdfAngleVar.h
    RooHistPdfVar.h
    RooPoly3Var.h
    RooPoly4Var.h
    RooSlimFitResult.h
    RooMultiPdf.h
)
# add a make rule for the dictionary
ROOT_GENERATE_DICTIONARY(${PROJECT_NAME}Dict
    ${CORE_DICTIONARY_SOURCES}
    LINKDEF coreLinkDef.h
)
# add a target calling the make rule for the dictionary
add_custom_target( ${PROJECT_NAME}Dict DEPENDS ${PROJECT_NAME}Dict.cxx ${PROJECT_NAME}Dict.h )
# add the dictionary to the list of library sources
SET(CORE_LIB_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict.cxx ${CORE_LIB_SOURCES})

######################################
#
# build core library
#
######################################

ADD_SHARED_LIBRARY( ${PROJECT_NAME}Components ${CORE_LIB_SOURCES} )
# add the ROOT dictionary to the list of dependencies
add_dependencies( ${PROJECT_NAME}Components ${PROJECT_NAME}Dict )
# add ROOT libraries to the linker command
TARGET_LINK_LIBRARIES( ${PROJECT_NAME}Components ${ROOT_LIBRARIES} ${FREETYPE_LIBRARIES} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Dict_rdict.pcm DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib )

######################################
#
# subdirectorys for the combiners
#
######################################

foreach( dir ${COMBINER_MODULES} )
    add_subdirectory(${dir})
endforeach()

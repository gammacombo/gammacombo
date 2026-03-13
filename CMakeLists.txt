cmake_minimum_required(VERSION 3.19)

# Project configuration -------------------------------------------------------

project(
  GammaCombo
  VERSION 1.4.0
  DESCRIPTION "LHCb frequentist combiner"
  HOMEPAGE_URL "https://gammacombo.github.io/"
  LANGUAGES CXX)

# Build Doxygen documentation by configuring cmake with -DBUILD_DOCUMENTATION=ON
option(BUILD_DOCUMENTATION
       "Create and install the HTML based API documentation (requires Doxygen)"
       OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set build destinations. Cannot use CMAKE_LIBRARY_OUTPUT_DIRECTORY due to
# root_generate_dictionary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
if(NOT LIBRARY_OUTPUT_DIRECTORY)
  set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()
# Set default for install destination
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${PROJECT_SOURCE_DIR})
endif()

# Find required packages ------------------------------------------------------

# Use Ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  set(CMAKE_CXX_LINKER_LAUNCHER ${CCACHE_PROGRAM})
  message(STATUS "Found Ccache: ${CCACHE_PROGRAM}")
else()
  message(STATUS "Could NOT find Ccache")
endif()

find_package(Freetype REQUIRED)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost 1.70 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Find ROOT (see https://root.cern/manual/integrate_root_into_my_cmake_project/,
# https://cliutils.gitlab.io/modern-cmake/chapters/packages/ROOT.html)
find_package(ROOT 6.30 CONFIG REQUIRED)
message(STATUS "Found ROOT: ${ROOT_INCLUDE_DIRS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O3 ${ROOT_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

if(BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/doxygen)
  doxygen_add_docs(doc core ALL
                   COMMENT "Generate doxygen pages for the core library")
endif()

# Build the core library ------------------------------------------------------

set(CORE_INC_DIR core/include)
include_directories(BEFORE ${CORE_INC_DIR})
configure_file(${CORE_INC_DIR}/VersionConfig.h.in VersionConfig.h)

set(CORE_LIB_SOURCES
    ./core/src/BatchScriptWriter.cpp
    ./core/src/CLInterval.cpp
    ./core/src/CLIntervalMaker.cpp
    ./core/src/CLIntervalPrinter.cpp
    ./core/src/Combiner.cpp
    ./core/src/ConfidenceContours.cpp
    ./core/src/Contour.cpp
    ./core/src/ControlPlots.cpp
    ./core/src/ExternalScanWrapper.cpp
    ./core/src/FileNameBuilder.cpp
    ./core/src/FitResultCache.cpp
    ./core/src/Fitter.cpp
    ./core/src/GammaComboEngine.cpp
    ./core/src/Graphviz.cpp
    ./core/src/LatexMaker.cpp
    ./core/src/MethodAbsScan.cpp
    ./core/src/MethodBergerBoosScan.cpp
    ./core/src/MethodCoverageScan.cpp
    ./core/src/MethodDatasetsPluginScan.cpp
    ./core/src/MethodDatasetsProbScan.cpp
    ./core/src/MethodPluginScan.cpp
    ./core/src/MethodProbScan.cpp
    ./core/src/OneMinusClPlot2d.cpp
    ./core/src/OneMinusClPlotAbs.cpp
    ./core/src/OneMinusClPlot.cpp
    ./core/src/OptParser.cpp
    ./core/src/ParameterCache.cpp
    ./core/src/Parameter.cpp
    ./core/src/ParameterEvolutionPlotter.cpp
    ./core/src/ParametersAbs.cpp
    ./core/src/PDF_Abs.cpp
    ./core/src/PDF_Datasets.cpp
    ./core/src/ProgressBar.cpp
    ./core/src/PullPlotter.cpp
    ./core/src/PValueCorrection.cpp
    ./core/src/RooBinned2DBicubicBase.cpp
    ./core/src/RooCrossCorPdf.cpp
    ./core/src/RooHistPdfAngleVar.cpp
    ./core/src/RooHistPdfVar.cpp
    ./core/src/RooMultiPdf.cpp
    ./core/src/RooPoly3Var.cpp
    ./core/src/RooPoly4Var.cpp
    ./core/src/RooSlimFitResult.cpp
    ./core/src/Rounder.cpp
    ./core/src/SharedArray.cpp
    ./core/src/ToyTree.cpp
    ./core/src/UtilsConfig.cpp
    ./core/src/Utils.cpp)

set(ROOT_REQUIRED_LIBS
    ROOT::Core
    ROOT::Graf
    ROOT::Gui
    ROOT::Hist
    ROOT::MathCore
    ROOT::Matrix
    ROOT::RIO
    ROOT::RooFit
    ROOT::Tree)

set(CORE_LIB "${PROJECT_NAME}Core")
add_library(${CORE_LIB} SHARED ${CORE_LIB_SOURCES})
target_link_libraries(${CORE_LIB} ${FREETYPE_LIBRARIES} ${ROOT_REQUIRED_LIBS})
# For finding ConfigVersion.h
target_include_directories(${CORE_LIB} PUBLIC ${CMAKE_BINARY_DIR})

# Add ROOT dictionaries to the core library, see:
# https://cliutils.gitlab.io/modern-cmake/chapters/packages/ROOT.html#dictionary-generation
# (but now RootNewMacros has been renamed RootMacros and is automatically
# included with find_package(ROOT))
set(CORE_DICTIONARY_SOURCES
    RooBinned2DBicubicBase.h
    RooCrossCorPdf.h
    RooHistPdfAngleVar.h
    RooHistPdfVar.h
    RooPoly3Var.h
    RooPoly4Var.h
    RooSlimFitResult.h
    RooMultiPdf.h)
root_generate_dictionary(G__${CORE_LIB} ${CORE_DICTIONARY_SOURCES} MODULE
                         ${CORE_LIB} LINKDEF core/include/coreLinkDef.h)

install(FILES ${CMAKE_BINARY_DIR}/lib${CORE_LIB}_rdict.pcm
        DESTINATION ${LIBRARY_OUTPUT_DIRECTORY})

# Add combiner modules to the build -------------------------------------------

# Set the list of output folders that should be created by each submodule
set(OUTPUT_FOLDERS
    root
    plots
    plots/C
    plots/cl
    plots/dot
    plots/eps
    plots/par
    plots/pdf
    plots/png
    plots/root
    plots/scanner)

# Get the list of combiner modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(combiners)

foreach(dir ${COMBINER_MODULES})
  add_subdirectory(${dir})
endforeach()

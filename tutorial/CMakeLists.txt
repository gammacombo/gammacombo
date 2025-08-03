# The combiner name should be the name of the subdirectory of the combiner.
set(COMBINER_NAME tutorial)

# Define all executables of this combiner. They must be located in the `main`
# subdirectory and end with `.cpp`.
set(COMBINER_EXECUTABLES
    ${COMBINER_NAME} cartesian tutorial_dataset
    tutorial_dataset_build_workspace tutorial_dataset_multipdf
    tutorial_dataset_multipdf_build_workspace)

# Configure if this combiner has custom ROOT objects
set(HAS_CUSTOMROOTOBJECTS FALSE)

# Define the custom ROOT objects
if(HAS_CUSTOMROOTOBJECTS)
  set(COMBINER_DICTIONARY_LINKDEF
      LinkDef.h # <-- change to name of LinkDef file
  )
  set(COMBINER_DICTIONARY_SOURCES Header.h # <-- add header files here
  )
endif()

# -----------------------------------------------------------------------------
# Set directory names and includes
# -----------------------------------------------------------------------------

set(COMBINER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMBINER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMBINER_MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/main)

include_directories(BEFORE ${CORE_INC_DIR} ${COMBINER_INCLUDE_DIR})

# -----------------------------------------------------------------------------
# Build the library
# -----------------------------------------------------------------------------

set(COMBINER_LIB_SOURCES
    ${COMBINER_SOURCE_DIR}/ParametersCartesian.cpp
    ${COMBINER_SOURCE_DIR}/ParametersTutorial.cpp
    ${COMBINER_SOURCE_DIR}/PDF_Cartesian.cpp
    ${COMBINER_SOURCE_DIR}/PDF_Circle.cpp
    ${COMBINER_SOURCE_DIR}/PDF_CrossCorAbs.cpp
    ${COMBINER_SOURCE_DIR}/PDF_CrossCor_GausA_vs_GausB.cpp
    ${COMBINER_SOURCE_DIR}/PDF_DatasetTutorial.cpp
    ${COMBINER_SOURCE_DIR}/PDF_Gaus.cpp
    ${COMBINER_SOURCE_DIR}/PDF_GausB.cpp
    ${COMBINER_SOURCE_DIR}/PDF_Gaus2d.cpp
    ${COMBINER_SOURCE_DIR}/PDF_rb.cpp)

set(COMBINER_LIB ${COMBINER_NAME}Components)
add_library(${COMBINER_LIB} SHARED ${COMBINER_LIB_SOURCES})
target_link_libraries(${COMBINER_LIB} ${CORE_LIB})
if(HAS_CUSTOMROOTOBJECTS)
  root_generate_dictionary(
    G__${COMBINER_LIB} ${COMBINER_DICTIONARY_SOURCES} MODULE ${COMBINER_LIB}
    LINKDEF ${COMBINER_DICTIONARY_LINKDEF})
endif()

# -----------------------------------------------------------------------------
# Build the executables
# -----------------------------------------------------------------------------

set(COMBINER_LIBS ${COMBINER_LIB} ${CORE_LIB})
foreach(exec ${COMBINER_EXECUTABLES})
  add_executable(${exec} ${COMBINER_MAIN_DIR}/${exec}.cpp)
  target_link_libraries(${exec} ${COMBINER_LIBS})
endforeach()

# -----------------------------------------------------------------------------
# Install the binaries from the build directory back into the project
# subdirectory
# -----------------------------------------------------------------------------

foreach(exec ${COMBINER_EXECUTABLES})
  install(PROGRAMS $<TARGET_FILE:${exec}>
          DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
endforeach()

# -----------------------------------------------------------------------------
# Create output directories
# -----------------------------------------------------------------------------

foreach(folder ${OUTPUT_FOLDERS})
  install(DIRECTORY DESTINATION ${COMBINER_NAME}/${folder})
endforeach()

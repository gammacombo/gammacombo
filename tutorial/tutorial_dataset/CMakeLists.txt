
# Set the combiner name
#  The name must be the name of the subdirectory of the combiner.
SET(COMBINER_NAME
	cartesian
)

# Define all executables of this combiner.
#  They have to be located in the main/ subdirectory, and
#  end with .cpp.
SET(COMBINER_EXECUTABLES
	${COMBINER_NAME}
	# tutorial2 # <-- add other execubables here
)

# Configure if this combiner has custom ROOT objects
SET(HAS_CUSTOMROOTOBJECTS FALSE)

# Define the custom ROOT objects
IF(HAS_CUSTOMROOTOBJECTS)
	SET(COMBINER_DICTIONARY_LINKDEF
		LinkDef.h # <-- change to name of LinkDef file
	)
	SET(COMBINER_DICTIONARY_SOURCES
		Header.h # <-- add header files here
	)
ENDIF()

######################################
#
# set directory names and includes
#
######################################

SET(COMBINER_INCLUDE_DIR ./include)
SET(COMBINER_SOURCE_DIR ./src)
SET(COMBINER_MAIN_DIR ./main)

# include directories
INCLUDE_DIRECTORIES( 
	BEFORE 
	${ROOT_INCLUDE_DIR}
	${CORE_INC_DIR}
	${COMBINER_INCLUDE_DIR}
)

######################################
#
# generate ROOT dictionaries
#
######################################

IF(HAS_CUSTOMROOTOBJECTS)
	# add a make rule for the dictionary
	ROOT_GENERATE_DICTIONARY(${COMBINER_NAME}Dict
		${COMBINER_DICTIONARY_SOURCES}
		LINKDEF ${COMBINER_DICTIONARY_LINKDEF}
	)
	# add a target calling the make rule for the dictionary
	add_custom_target( ${COMBINER_NAME}Dict DEPENDS ${COMBINER_NAME}Dict.cxx ${COMBINER_NAME}Dict.h )
ENDIF()

######################################
#
# build the library
#
######################################

AUX_SOURCE_DIRECTORY( ${COMBINER_SOURCE_DIR} COMBINER_LIB_SOURCES )
IF(HAS_CUSTOMROOTOBJECTS)
	# add the ROOT dictionary to the list of library sources
	SET(COMBINER_LIB_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${COMBINER_NAME}Dict.cxx ${COMBINER_LIB_SOURCES})
ENDIF()
ADD_SHARED_LIBRARY( ${COMBINER_NAME}Components ${COMBINER_LIB_SOURCES} )
IF(HAS_CUSTOMROOTOBJECTS)
	# add the ROOT dictionary to the list of dependencies
	add_dependencies( ${COMBINER_NAME}Components ${COMBINER_NAME}Dict )
ENDIF()
TARGET_LINK_LIBRARIES( ${COMBINER_NAME}Components ${ROOT_LIBRARIES} ${PROJECT_NAME}Components )

######################################
#
# build the execuables
#
######################################

SET(COMBINER_LIBS
	${COMBINER_NAME}Components  
	${PROJECT_NAME}Components
)
foreach( exec ${COMBINER_EXECUTABLES} )
	add_executable( ${exec} ${COMBINER_MAIN_DIR}/${exec}.cpp )
	target_link_libraries( ${exec} ${COMBINER_LIBS} )
endforeach()

######################################
#
# install the binaries from the build directory back into the project subdirectory
#
######################################

foreach( exec ${COMBINER_EXECUTABLES} )
	INSTALL( PROGRAMS ../build/bin/${exec}
		DESTINATION ${COMBINER_NAME}/bin/
	)
endforeach()

######################################
#
# create gammacombo output directories
#
######################################

install(DIRECTORY DESTINATION ${COMBINER_NAME}/root         )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots        )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/C      )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/cl     )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/dot    )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/eps    )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/par    )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/pdf    )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/png    )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/root   )
install(DIRECTORY DESTINATION ${COMBINER_NAME}/plots/scanner)


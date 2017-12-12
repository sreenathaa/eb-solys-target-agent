cmake_minimum_required(VERSION 2.8.5)

PROJECT ( DltMonitorPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
#AMD64	     x		|	x	|	x	|
set(dlt_monitor_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 0 0" PARENT_SCOPE)

set(dlt_monitor_Capabilities "\t\t<${plugin}>\n\t\t\t<Path\ attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
        \n\t\t\t<!-- Set filter based on Application ID (MOST) and Context ID (IN) -->
		\n\t\t\t<!-- Add your config Item here: <dltFilter attr=\"MOST IN\"/> -->
		\n\t\t\t<!-- Set filter based only on Application ID (NAVI) all contexts associated with the Application ID will be selected-> -->
		\n\t\t\t<!-- Add your config Item here: <dltFilter attr=\"NAVI &\#45\;&\#45\;&\#45\;&\#45\;\"/> -->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t</${plugin}>" PARENT_SCOPE)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -std=c++11")
ADD_DEFINITIONS("-Wall -Wno-unknown-pragmas -Wno-write-strings")
IF (CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
ENDIF()

include(FindProtobuf)
find_package(Protobuf REQUIRED)
find_package(Poco REQUIRED Foundation Util Net XML JSON)

INCLUDE(FindPkgConfig)
pkg_check_modules(DLT REQUIRED automotive-dlt)
		    
# protobuf-related actions
SET (PROTOCOL_DEFINITION_SOURCE_DIR 	${PROJECT_SOURCE_DIR}/../../protocol-definitions)
SET (PROTOCOL_DEFINITION_INPUT_FILES 	${PROTOCOL_DEFINITION_SOURCE_DIR}/target_agent_prot_most_spy.proto)				    

SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)

if (${PROTO_REMOVE_GEN_FILES})
    file (GLOB remove_file "${CMAKE_CURRENT_SOURCE_DIR}/gen/*.*")
    list(LENGTH remove_file RES_LEN)
        if(NOT ${RES_LEN} EQUAL "0")
                file (REMOVE ${remove_file})
        endif()
endif()

foreach(PROTO_FILE ${PROTOCOL_DEFINITION_INPUT_FILES})
	#get the name of the file without extension
	get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

	#add the generated files
	set (PROTO_GEN_SOURCE_FILES ${PROTO_GEN_SOURCE_FILES}
	${PROTOBUF_GEN_OUTPUT_DIR}/${PROTO_NAME}.pb.cc)

	set (PROTO_GEN_HEADER_FILES ${PROTO_GEN_HEADER_FILES}
	${PROTOBUF_GEN_OUTPUT_DIR}/${PROTO_NAME}.pb.h)
	

endforeach(PROTO_FILE ${PROTOCOL_DEFINITION_INPUT_FILES})

execute_process(
        COMMAND ${Protobuf_PROTOC_EXECUTABLE} --proto_path=${PROTOCOL_DEFINITION_SOURCE_DIR} --cpp_out=${PROTOBUF_GEN_OUTPUT_DIR} ${PROTOCOL_DEFINITION_INPUT_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)



MESSAGE( STATUS "DLT_INCLUDE_DIRS:         " ${DLT_INCLUDE_DIRS} )
MESSAGE( STATUS "DLT_LIBRARIES:         " ${DLT_LIBRARIES} )
INCLUDE_DIRECTORIES ( 
    ${DLT_INCLUDE_DIRS}
    ${DLT_LIBRARIES} 
    #    ${POCO_INCLUDE_DIR} 
    ${PROTOBUF_INCLUDE_DIR} 
    ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
    ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
    inc
    gen)
			 
LINK_DIRECTORIES  (${POCO_LIB_PATH} ${PROTO_BUF_LIB_PATH} ${COMMON_PLUGIN_INTERFACE_LIB_PATH})

SET (DLT_MONITOR_PLUGIN_SRC src/dlt-monitor-plugin.cpp
src/dlt-client-activity.cpp
)


aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gen PROTO_GLUE_CODE)

ADD_LIBRARY (dlt-monitor-plugin SHARED ${DLT_MONITOR_PLUGIN_SRC} ${PROTO_GLUE_CODE})

TARGET_LINK_LIBRARIES(dlt-monitor-plugin 
    ${PROTOBUF_LIBRARY} 
    Poco::Foundation
    Poco::Util
    Poco::Net
    Poco::XML
    ${DLT_LIBRARIES} 
    pthread
    )	


INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/${BUILD_FOLDER_NAME}/${LINUX_LIB_PREFIX}dlt-monitor-plugin${DEBUG_SUFFIX}${LIBRARY_EXTENSION} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${TA_ENABLE_TESTING})
  INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR})
  
  SET(DLT_MONITOR_PLUGIN_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/dlt_monitor_plugin_test.cpp)

  ADD_EXECUTABLE(dlt-monitor-plugin${DEBUG_SUFFIX}-test ${DLT_MONITOR_PLUGIN_TEST_LIB_SRC} )

  TARGET_LINK_LIBRARIES(
	  dlt-monitor-plugin${DEBUG_SUFFIX}-test
	  dlt-monitor-plugin${DEBUG_SUFFIX}
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  pthread )
  
  add_test(
    NAME dlt-monitor-plugin${DEBUG_SUFFIX}-test
    COMMAND dlt-monitor-plugin${DEBUG_SUFFIX}-test
  )
ENDIF()

cmake_minimum_required(VERSION 2.8)

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

set(dlt_monitor_Capabilities "\t\t<${plugin}>\n\t\t\t<Path\ attr=\"./plugins/${file_name}\"/>
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
INCLUDE(FindPkgConfig)

pkg_check_modules(DLT REQUIRED automotive-dlt)
		    
		    
MESSAGE( STATUS "DLT_INCLUDE_DIRS:         " ${DLT_INCLUDE_DIRS} )
MESSAGE( STATUS "DLT_LIBRARIES:         " ${DLT_LIBRARIES} )
INCLUDE_DIRECTORIES (
  ${DLT_INCLUDE_DIRS}
		    ${DLT_LIBRARIES} 
${POCO_INCLUDE_DIR} 
		     ${PROTO_BUF_INCLUDE_DIR} 
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     inc
		     gen)
			 
LINK_DIRECTORIES  (${POCO_LIB_PATH} ${PROTO_BUF_LIB_PATH} ${COMMON_PLUGIN_INTERFACE_LIB_PATH})

SET (DLT_MONITOR_PLUGIN_SRC src/dlt-monitor-plugin.cpp
src/dlt-client-activity.cpp
)

SET(DLT_GEN_FILES 
gen/target_agent_prot_dlt_monitor_plugin.pb.cc
gen/target_agent_prot_most_spy.pb.cc
)


SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)
FILE(GLOB PROTO_GEN_SOURCE_FILES RELATIVE "${PROTOBUF_GEN_OUTPUT_DIR}"   "${PROTOBUF_GEN_OUTPUT_DIR}/*.cc")

ADD_LIBRARY (dlt-monitor-plugin SHARED ${DLT_MONITOR_PLUGIN_SRC} ${DLT_GEN_FILES})

TARGET_LINK_LIBRARIES(dlt-monitor-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} ${POCO_LIB_NET} ${DLT_LIBRARIES} pthread)	


INSTALL(TARGETS dlt-monitor-plugin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${TA_ENABLE_TESTING})
  INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR})
  
  SET(DLT_MONITOR_PLUGIN_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/dlt_monitor_plugin_test.cpp)

  ADD_EXECUTABLE(dlt-monitor-plugin-test ${DLT_MONITOR_PLUGIN_TEST_LIB_SRC} )

  TARGET_LINK_LIBRARIES(
	  dlt-monitor-plugin-test
	  dlt-monitor-plugin
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  pthread )
  
  add_test(
    NAME dlt-monitor-plugin-test
    COMMAND dlt-monitor-plugin-test
  )
ENDIF()
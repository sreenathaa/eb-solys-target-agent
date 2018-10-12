cmake_minimum_required(VERSION 2.8)

PROJECT ( resource-monitor )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
#AMD64	     x		|	x	|	x	|
set(resource_monitor_win7_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 1 0" PARENT_SCOPE)

SET(PLUGIN_NAME "resource-monitor-plugin")
set(resource_monitor_win7_Capabilities "\t\t<${PLUGIN_NAME}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${PLUGIN_NAME}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
		\n\t\t\t<!-- Resource Monitor Sampling Rate: the recommended range is [50ms,5000ms] default is 1000ms -->
		\n\t\t\t<!-- Setting a sampling rate outside this interval may lead to unreliable measurements-->
		\n\t\t\t<!-- Uncomment the line below and adapt it according to your needs-->
		\n\t\t\t<samplingRate attr=\"1000\"/>
		\n\t\t</${PLUGIN_NAME}>" PARENT_SCOPE)

#configure_msvc_runtime("dynamic")

INCLUDE_DIRECTORIES (${POCO_INCLUDE_DIR} 
		     ${PROTO_BUF_INCLUDE_DIR} 
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     inc
		     gen
             )
			 
LINK_DIRECTORIES  (${POCO_LIB_PATH} ${PROTO_BUF_LIB_PATH} ${COMMON_PLUGIN_INTERFACE_LIB_PATH})

SET (RESOURCE_MONITOR_PLUGIN_SRC
				src/resource-monitor-plugin.cpp
                src/resource-monitor-scheduler.cpp
                src/resource-monitor-utils.cpp
	)

SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)
FILE(GLOB PROTO_GEN_SOURCE_FILES RELATIVE "${PROTOBUF_GEN_OUTPUT_DIR}"   "${PROTOBUF_GEN_OUTPUT_DIR}/*.pb.cc")
ADD_LIBRARY (resource-monitor-plugin SHARED ${RESOURCE_MONITOR_PLUGIN_SRC} gen/${PROTO_GEN_SOURCE_FILES})

TARGET_LINK_LIBRARIES(resource-monitor-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} psapi.lib)

INSTALL(TARGETS resource-monitor-plugin
DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${TA_LOCAL_ENABLE_TESTING})
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
 # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
  INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR})
  
  SET(RESOURCE_MONITOR_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/resource-monitor-plugin-test.cpp)

  ADD_EXECUTABLE(resource-monitor-plugin${DEBUG_SUFFIX}-test ${RESOURCE_MONITOR_TEST_LIB_SRC})

  TARGET_LINK_LIBRARIES(
	  resource-monitor-plugin${DEBUG_SUFFIX}-test
	  ${PROTO_BUF_LIB}
	  ${POCO_LIB_FOUNDATION}
	  resource-monitor-plugin${DEBUG_SUFFIX}
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  pthread
	  )
  
  add_test(
    NAME resource-monitor-plugin${DEBUG_SUFFIX}-test
    COMMAND resource-monitor-plugin${DEBUG_SUFFIX}-test
  )
ENDIF()


cmake_minimum_required(VERSION 2.8)

PROJECT ( ThreadStatsPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
#AMD64	     x		|	x	|	x	|
set(linux_app_stats_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 0 0" PARENT_SCOPE)

set(linux_app_stats_Capabilities "\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
		\n\t\t\t<!-- the query messagemessage-->
		\n\t\t\t<Query attr=\"$06 Bonjour\"/>
		\n\t\t\t<!-- Reolution: how often shall the data be queried from the observed process-->
		\n\t\t\t<samplingRate attr=\"100\"/>
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t</${plugin}>" PARENT_SCOPE)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -std=c++11")
ADD_DEFINITIONS("-Wall -Wno-unknown-pragmas -Wno-write-strings")
IF (CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
ENDIF()

INCLUDE_DIRECTORIES(
			 ${POCO_INCLUDE_DIR} 
		     ${PROTO_BUF_INCLUDE_DIR} 
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     inc
		     gen)
			 
LINK_DIRECTORIES  (${POCO_LIB_PATH} ${PROTO_BUF_LIB_PATH})

SET (LINUX_APP_STATS_PLUGIN_SRC
			src/linux_app_stats_plugin.cpp
			src/stats_collector.cpp
			src/socket_descriptor.cpp
			src/periodic_task.cpp
)


SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)
FILE(GLOB PROTO_GEN_SOURCE_FILES RELATIVE "${PROTOBUF_GEN_OUTPUT_DIR}"   "${PROTOBUF_GEN_OUTPUT_DIR}/*.cc")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gen PROTO_GLUE_CODE)

ADD_LIBRARY (linux-app-stats-plugin SHARED ${LINUX_APP_STATS_PLUGIN_SRC}  ${PROTO_GLUE_CODE})

TARGET_LINK_LIBRARIES(linux-app-stats-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} ${POCO_LIB_NET} pthread)	

INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/${BUILD_FOLDER_NAME}/${LINUX_LIB_PREFIX}linux-app-stats-plugin${DEBUG_SUFFIX}${LIBRARY_EXTENSION} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${TA_ENABLE_TESTING})
 INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  
  SET(LINUX_APP_STATS_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/linux_app_stats_plugin_test.cpp)

  ADD_EXECUTABLE(linux-app-stats-plugin${DEBUG_SUFFIX}-test ${LINUX_APP_STATS_TEST_LIB_SRC})

  TARGET_LINK_LIBRARIES(
	  linux-app-stats-plugin${DEBUG_SUFFIX}-test
	  ${PROTO_BUF_LIB}
	  ${POCO_LIB_FOUNDATION}
	  linux-app-stats-plugin${DEBUG_SUFFIX}
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  pthread
	  )
  
  add_test(
    NAME linux-app-stats-plugin${DEBUG_SUFFIX}-test
    COMMAND linux-app-stats-plugin${DEBUG_SUFFIX}-test
    )
ENDIF()
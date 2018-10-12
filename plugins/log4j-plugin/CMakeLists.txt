cmake_minimum_required(VERSION 2.8)

PROJECT ( log4jPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
set(log4j_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 1 0" PARENT_SCOPE)

set(log4j_Capabilities "\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
		\n\t\t\t<!-- Add your config Item here: <log4jLogFile attr=\"/path/to/logfile/log4jLogFile.log\"/> -->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t</${plugin}>" PARENT_SCOPE)


INCLUDE_DIRECTORIES(
			 ${POCO_INCLUDE_DIR}
		     ${PROTO_BUF_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     inc
		     gen)

LINK_DIRECTORIES  (${POCO_LIB_PATH} ${PROTO_BUF_LIB_PATH})

SET (log4jPLUGIN_SRC
			src/log4jPlugin.cpp
			src/log4jReader.cpp
)

SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)
FILE(GLOB PROTO_GEN_SOURCE_FILES ABSOLUTE "${PROTOBUF_GEN_OUTPUT_DIR}"   "${PROTOBUF_GEN_OUTPUT_DIR}/*.pb.cc")


ADD_LIBRARY (log4j-plugin SHARED ${log4jPLUGIN_SRC}  ${PROTO_GEN_SOURCE_FILES})


TARGET_LINK_LIBRARIES(log4j-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} ${POCO_LIB_NET})

INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/${BUILD_FOLDER_NAME}/${LINUX_LIB_PREFIX}log4j-plugin${DEBUG_SUFFIX}${LIBRARY_EXTENSION} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${TA_LOCAL_ENABLE_TESTING})
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -std=c++11")
  
  INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR})
  
  SET(SOCKET_READER_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/log4jReaderTest.cpp)

  ADD_EXECUTABLE(log4j-plugin${DEBUG_SUFFIX}-test ${SOCKET_READER_TEST_LIB_SRC})

  TARGET_LINK_LIBRARIES(
	  log4j-plugin${DEBUG_SUFFIX}-test
	  ${PROTO_BUF_LIB}
	  ${POCO_LIB_FOUNDATION}
	  log4j-plugin${DEBUG_SUFFIX}
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  )
  
  add_test(
    NAME log4j-plugin${DEBUG_SUFFIX}-test
    COMMAND log4j-plugin${DEBUG_SUFFIX}-test
  )
ENDIF()
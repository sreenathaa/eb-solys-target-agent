cmake_minimum_required(VERSION 2.8)

PROJECT ( SocketReaderPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
#AMD64	     x		|	x	|	x	|
set(socket_reader_platformSupportMatrix  "1 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 1 0" PARENT_SCOPE)

set(socket_reader_Capabilities "\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
		\n\t\t\t<!-- Add your config Item here: <portNumber attr=\"1111\"/> -->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t</${plugin}>" PARENT_SCOPE)


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

SET (SOCKET_READER_PLUGIN_SRC 
			src/socket_reader_plugin.cpp
			src/socket_reader_activity.cpp
)


aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gen PROTO_GLUE_CODE)

ADD_LIBRARY (socket-reader-plugin SHARED ${SOCKET_READER_PLUGIN_SRC}  ${PROTO_GLUE_CODE})


TARGET_LINK_LIBRARIES(socket-reader-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} ${POCO_LIB_NET})	

INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/${BUILD_FOLDER_NAME}/${LINUX_LIB_PREFIX}socket-reader-plugin${DEBUG_SUFFIX}${LIBRARY_EXTENSION} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${TA_ENABLE_TESTING})
  INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR})
  
  SET(SOCKET_READER_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/socket_reader_plugin_test.cpp)

  ADD_EXECUTABLE(socket-reader-plugin${DEBUG_SUFFIX}-test ${SOCKET_READER_TEST_LIB_SRC})

  TARGET_LINK_LIBRARIES(
	  socket-reader-plugin${DEBUG_SUFFIX}-test
	  ${PROTO_BUF_LIB}
	  ${POCO_LIB_FOUNDATION}
	  socket-reader-plugin${DEBUG_SUFFIX}
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  )
  
  add_test(
    NAME socket-reader-plugin${DEBUG_SUFFIX}-test
    COMMAND socket-reader-plugin${DEBUG_SUFFIX}-test
  )
ENDIF()
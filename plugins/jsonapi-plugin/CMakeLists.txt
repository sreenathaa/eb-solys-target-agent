cmake_minimum_required(VERSION 2.8)

PROJECT ( jsonapiPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
set(jsonapi_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0" PARENT_SCOPE)

set(jsonapi_Capabilities "\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
		\n\t\t\t<!-- Add your config Item here: <portNumber attr=\"1111\"/> -->
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

SET (jsonapiPLUGIN_SRC
			src/jsonapiPlugin.cpp
)

SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)
FILE(GLOB PROTO_GEN_SOURCE_FILES ABSOLUTE "${PROTOBUF_GEN_OUTPUT_DIR}"   "${PROTOBUF_GEN_OUTPUT_DIR}/*.pb.cc")


ADD_LIBRARY (jsonapi-plugin SHARED ${jsonapiPLUGIN_SRC}  ${PROTO_GEN_SOURCE_FILES})


TARGET_LINK_LIBRARIES(jsonapi-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} ${POCO_LIB_NET})

INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/${BUILD_FOLDER_NAME}/${LINUX_LIB_PREFIX}jsonapi-plugin${DEBUG_SUFFIX}${LIBRARY_EXTENSION} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${LOCAL_ENABLE_TESTING})
 ##add the logic corresponding to gmock/gtest tests here
ENDIF()

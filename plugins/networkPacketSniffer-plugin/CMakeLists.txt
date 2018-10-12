cmake_minimum_required(VERSION 2.8)

PROJECT ( networkPacketSnifferPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
set(networkPacketSniffer_platformSupportMatrix  "1 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 1 0" PARENT_SCOPE)

list(APPEND CMAKE_MODULE_PATH /home/yocto/rpi3/libtins/install/lib/pkgconfig/ )
set(networkPacketSniffer_Capabilities "\n<!-- First Configure Valid Interface and then activate plugin -->
		\n\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/> 
		\n\t\t\t<NetworkInterface attr=\"lo\"/> 
		\n\t\t\t<PortNr attr=\"1239\"/>
		\n\t\t\t<payloadType attr=\"SOMEIP\"/>  
		\n\t\t\t<ProtocolType  attr=\"UDP\"/>  
		\n\t\t</${plugin}>\n" PARENT_SCOPE) 

find_package(PkgConfig)
find_package(libtins 3.4)
if (${libtins_FOUND})
    MESSAGE(STATUS "libtins found via CMake")
else (${libtins_FOUND})
    MESSAGE(STATUS "libtins not found via CMake, trying pkg-config")
    pkg_search_module(libtins REQUIRED libtins)
endif (${libtins_FOUND})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
INCLUDE_DIRECTORIES(
			 ${LIBTINS_INCLUDE_DIRS}
			 ${POCO_INCLUDE_DIR}
		     ${PROTO_BUF_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     inc
		     gen)

LINK_DIRECTORIES  (${POCO_LIB_PATH} ${PROTO_BUF_LIB_PATH})

if (NOT WIN32)
	SET(PORT_MAPPER		src/utils/port_owner_lookup.c)
endif (NOT WIN32)
	
SET (networkPacketSnifferPLUGIN_SRC
			src/networkPacketCollector.cpp
			src/networkPacketSnifferPlugin.cpp
			${PORT_MAPPER}
)

SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)
FILE(GLOB PROTO_GEN_SOURCE_FILES ABSOLUTE "${PROTOBUF_GEN_OUTPUT_DIR}"   "${PROTOBUF_GEN_OUTPUT_DIR}/*.pb.cc")


ADD_LIBRARY (networkPacketSniffer-plugin SHARED ${networkPacketSnifferPLUGIN_SRC}  ${PROTO_GEN_SOURCE_FILES})

TARGET_LINK_LIBRARIES(networkPacketSniffer-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} ${POCO_LIB_NET} ${LIBTINS_LIBRARIES})
INSTALL(TARGETS networkPacketSniffer-plugin ARCHIVE DESTINATION lib LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/ COMPONENT library) 


IF(${LOCAL_ENABLE_TESTING})
 ##add the logic corresponding to gmock/gtest tests here
ENDIF()

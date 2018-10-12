cmake_minimum_required(VERSION 2.8)

PROJECT ( dbusCmdCtrlPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
#AMD64	     x		|	x	|	x	|
set(dbusCmdCtrl_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 0 0" PARENT_SCOPE)

set(dbusCmdCtrl_Capabilities "\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
        \n\t\t\t<!-- Due to a limitation in the current version this plugin/> -->
		\n\t\t\t<!-- should appear after the dbus-monitor plugin in the conf.xml -->
		\n\t\t\t<!-- PLEASE MOVE THE PLUGIN ACCORDINGLY-->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t</${plugin}>" PARENT_SCOPE)

# Find DBUS
find_package(PkgConfig REQUIRED)
pkg_search_module(DBUS dbus-1)
if(DBUS_FOUND)
   # nop
else()
set( DBUS_INCLUDE_DIRS ${CMAKE_SYSROOT_PATH}/usr/include/dbus-1.0
                       ${CMAKE_SYSROOT_PATH}/usr/lib/dbus-1.0/include
                       ${CMAKE_SYSROOT_PATH}/lib/dbus-1.0/include
                       /usr/lib/i386-linux-gnu/dbus-1.0/include
   )
endif()

INCLUDE_DIRECTORIES(
			 ${POCO_INCLUDE_DIR}
		     ${PROTO_BUF_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		      ${DBUS_INCLUDE_DIRS}
		     inc
		     gen)

LINK_DIRECTORIES  (${POCO_LIB_PATH} ${PROTO_BUF_LIB_PATH})

SET (dbusCmdCtrlPLUGIN_SRC
			src/dbusCmdCtrl_plugin.cpp
)

SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)
FILE(GLOB PROTO_GEN_SOURCE_FILES ABSOLUTE "${PROTOBUF_GEN_OUTPUT_DIR}"   "${PROTOBUF_GEN_OUTPUT_DIR}/*.pb.cc")


ADD_LIBRARY (dbusCmdCtrl-plugin SHARED ${dbusCmdCtrlPLUGIN_SRC}  ${PROTO_GEN_SOURCE_FILES})


TARGET_LINK_LIBRARIES(dbusCmdCtrl-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} ${POCO_LIB_NET} dbus-1)

INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/${BUILD_FOLDER_NAME}/${LINUX_LIB_PREFIX}dbusCmdCtrl-plugin${DEBUG_SUFFIX}${LIBRARY_EXTENSION} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${LOCAL_ENABLE_TESTING})
 ##add the logic corresponding to gmock/gtest tests here
ENDIF()
cmake_minimum_required(VERSION 3.0)

PROJECT ( solys-agent )

string(TIMESTAMP TS "%Y-%m-%d %H:%M:%S")
SET(TA_VERSION_INFO "Developer build from ${TS}" CACHE STRING "Target Agent Version related information")

set (git_op "clone")
set (git_show_progress "--progress")
set (poco_repository "https://github.com/emma3826/poco.git")
execute_process(COMMAND git ${git_op} ${poco_repository}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty
  RESULT_VARIABLE result
)
set (git_op "checkout")
set (git_branch "poco-1.6.1")
message(STATUS "git cmd result: ${result}")
execute_process(COMMAND git ${git_op} ${git_branch}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty/poco
  RESULT_VARIABLE result
)
message(STATUS "git cmd result: ${result}")
set (git_op "clone")
set (protobuf_repository "https://github.com/emma3826/protobuf.git")
execute_process(COMMAND git ${git_op} ${protobuf_repository}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty
  RESULT_VARIABLE result
)
set (git_op "checkout")
set (protobuf_repository "cmake_integration")
execute_process(COMMAND git ${git_op} ${protobuf_repository}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty/protobuf
  RESULT_VARIABLE result
)
### include
include(ExternalProject)
include(utils/PlatformAbstractionCMakeLists.txt)
#set (CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/deploy CACHE STRINGS "cmake install path")
macro(force_static_build)
  if(MSVC)
    set(MSVC_RUNTIME "static")
	
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
	foreach(variable ${variables})
		if(${variable} MATCHES "/MD")
		  string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
		endif()
	endforeach()
  endif()
endmacro()
force_static_build()

if (WIN32)
    set (PROTO_EXE_SUFFIX ".exe")
    MESSAGE(STATUS "Build a visual studio based solution")
    add_definitions(-D_UNICODE;-DUNICODE )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "QNX")
	set (PROTO_EXE_SUFFIX ".exe")
else(WIN32)
    set (PROTO_EXE_SUFFIX "")    
endif(WIN32)

set (PROTO_PATH_TO_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty/protobuf/bin/)
set (PROTO_COMMAND ${PROTO_PATH_TO_GENERATOR}/protoc${PROTO_EXE_SUFFIX})
option (PROTO_REMOVE_GEN_FILES "remove all generated files before build" OFF) 
option (PROTO_BUILD_SKIP "skip protobuf build once built" OFF)


if (NOT WIN32)

    set (PROTO_BUILD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty/protobuf/ CACHE STRING
      "given path to build directory" FORCE)
    
    option (PROTO_CLEAN_BUILD_DIRECTORIES "clean all build directory for new build" OFF)
         
    set (PROTO_BUILD_DIR  protoBuild )

    set(PROTO_LIB_NAME  "protobuf" CACHE STRING "BaseName chosen by ")
    set_property(CACHE PROTO_LIB_NAME PROPERTY STRINGS protobuf libprotobuf)

    set (PROTO_INSTALL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty/protobuf/ CACHE STRING
      "given path to install directory" FORCE)
   
    set (PROTO_BUILD_OPTION  "--prefix=${PROTO_INSTALL_PATH}" CACHE STRING "given options to build protoBuf")
    
    separate_arguments(PROTO_BUILD_OPTION)
    
    foreach(element ${PROTO_BUILD_OPTION})
      list(APPEND tmp ${element})
    endforeach(element ${PROTO_BUILD_OPTION})

    set (PROTO_BUILD_OPTION  ${tmp})

    if (${PROTO_CLEAN_BUILD_DIRECTORIES})
        file (GLOB_RECURSE remove_file "${PROTO_BUILD_PATH}${PROTO_BUILD_DIR}/*")
        list(LENGTH remove_file RES_LEN)
        if(NOT ${RES_LEN} EQUAL "0")
                file (REMOVE ${remove_file})
        endif()
        
        file (GLOB_RECURSE remove_file "${PROTO_INSTALL_PATH}/*")
        list(LENGTH remove_file RES_LEN)
        if(NOT ${RES_LEN} EQUAL "0")
                file (REMOVE ${remove_file})
        endif()
    endif()
    
    set (PROTO_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/target-agent/extern/3rdparty/protobuf/lib/scratchBuild/include)
    option (PROTO_CLEAN_BUILD_DIRECTORIES "clean all build directory for new build" OFF)

endif(NOT WIN32)


add_subdirectory(${CMAKE_SOURCE_DIR}/target-agent/extern)

##POCO retrieving the properties for the above mentioned modules
GET_PROPERTY(POCO_LIB_PATH GLOBAL PROPERTY POCO_LIB_PATH)
GET_PROPERTY(POCO_BIN_PATH GLOBAL PROPERTY POCO_BIN_PATH)
GET_PROPERTY(POCO_INCLUDE_DIR GLOBAL PROPERTY POCO_INCLUDE_DIR)
GET_PROPERTY(POCO_LIB_FOUNDATION GLOBAL PROPERTY POCO_LIB_FOUNDATION)
GET_PROPERTY(POCO_LIB_CRYPTO GLOBAL PROPERTY POCO_LIB_CRYPTO)
GET_PROPERTY(POCO_LIB_UTIL GLOBAL PROPERTY POCO_LIB_UTIL)
GET_PROPERTY(POCO_LIB_NET GLOBAL PROPERTY POCO_LIB_NET)
GET_PROPERTY(POCO_LIB_XML GLOBAL PROPERTY POCO_LIB_XML)
GET_PROPERTY(POCO_LIB_JASON GLOBAL PROPERTY POCO_LIB_JASON)
GET_PROPERTY(POCO_SHARED_LIB_VERSION GLOBAL PROPERTY POCO_SHARED_LIB_VERSION)
SET(POCO_LIBS_NAME "${POCO_LIB_UTIL} ${POCO_LIB_NET}  ${POCO_LIB_XML}  ${POCO_LIB_FOUNDATION} ${POCO_LIB_JASON}  ${POCO_LIB_UTIL}")

#get_target_property(inclu ${POCO_LIB_NET} INCLUDE_DIRECTORIES)

##PROTOBUF retrieving the properties for the above mentioned modules
GET_PROPERTY(PROTO_BUF_INCLUDE_DIR GLOBAL PROPERTY PROTO_BUF_INCLUDE_DIR)
GET_PROPERTY(PROTO_BUF_LIB_PATH GLOBAL PROPERTY PROTO_BUF_LIB_PATH)
GET_PROPERTY(PROTO_BUF_LIB GLOBAL PROPERTY PROTO_BUF_LIB)
GET_PROPERTY(PROTO_BUF_SHARED_LIB_VERSION GLOBAL PROPERTY PROTO_BUF_SHARED_LIB_VERSION)


#gTest
GET_PROPERTY(GTEST_INCLUDE_DIR GLOBAL PROPERTY GTEST_INCLUDE_DIR)
GET_PROPERTY(GTEST_LIB GLOBAL PROPERTY GTEST_LIB)
#gMock
GET_PROPERTY(GMOCK_INCLUDE_DIR GLOBAL PROPERTY GMOCK_INCLUDE_DIR)
GET_PROPERTY(GMOCK_LIB GLOBAL PROPERTY GMOCK_LIB)

file (STRINGS "${CMAKE_SOURCE_DIR}/protocol-definitions/version" BUILD_NUMBER)

configure_file(${CMAKE_SOURCE_DIR}/utils/target_agent_version.hpp.in ${CMAKE_SOURCE_DIR}/target-agent/modules/target-agent-runtime/gen/target_agent_version.hpp)

MESSAGE(STATUS, "Build number ${BUILD_NUMBER}")

#is always included to the build
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/target-agent)

#retrieve the subdirectory list
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

function(load_plugins)

### functions 
function(isConfigSupported platformCompatibilityMatrix isSupported)
  
        string(TOUPPER ${CMAKE_SYSTEM_PROCESSOR} U_SYSTEM_PROC)
        string(TOUPPER ${CMAKE_SYSTEM_NAME} U_SYSTEM_NAME)      
        MESSAGE(STATUS,"U_SYSTEM_PROC ${U_SYSTEM_PROC}")
        MESSAGE(STATUS,"U_SYSTEM_NAME ${U_SYSTEM_NAME}")
        SET(QNX_CLMN_ID			0)
        SET(WINDOWS_CLMN_ID 	    1)
        SET(LINUX_CLMN_ID 		2)

        SET(ARM_LINE_ID 			0)
        SET(I686_LINE_ID 			1)
        SET(X86_64_LINE_ID 		2)
        SET(SH2_LINE_ID 			3)
        SET(SH4_LINE_ID 			4)
        SET(AMD64_LINE_ID 		5)

        string(REPLACE "|" ";" ROWS ${platformCompatibilityMatrix})
        list(LENGTH ROWS len)

        list (GET ROWS ${${U_SYSTEM_PROC}_LINE_ID} ARCH_LINE)

        string(REPLACE " " ";" COLOMNS ${ARCH_LINE})
        list(LENGTH COLOMNS len)


        list (GET COLOMNS ${${U_SYSTEM_NAME}_CLMN_ID} INTERSECTION)


        if(${INTERSECTION} STREQUAL "0")
          set(${isSupported} FALSE PARENT_SCOPE)
        else()
          set(${isSupported} TRUE PARENT_SCOPE)
        endif()

      message(STATUS "compatibility matrix for [${CMAKE_SYSTEM_NAME},${CMAKE_SYSTEM_PROCESSOR}] shows:: ${INTERSECTION}")
      
endfunction(isConfigSupported)


SUBDIRLIST(SUBDIRS ${CMAKE_SOURCE_DIR}/plugins)
 FOREACH(plugin ${SUBDIRS})
	MESSAGE(STATUS "::::::::::::::::::::::::::::::::::::::::::::Available Plugins:::::::::::::::::::::::::::::::::::::::::::: ${plugin}")
	
	SET(qualified_plugin_name TA_PLUGIN_${plugin})
	
	IF(CMAKE_BUILD_TYPE MATCHES Debug)
	       SET(DEBUG_TOKEN "d")
    	ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)
	SET(file_name ${CMAKE_SHARED_LIBRARY_PREFIX}${plugin}${DEBUG_TOKEN}${CMAKE_SHARED_LIBRARY_SUFFIX})
	SET(${qualified_plugin_name} OFF CACHE BOOL "Build latest version of ${qualified_plugin_name}")
	
	MESSAGE(STATUS "Build latest version of ${qualified_plugin_name}")
	
	if(${qualified_plugin_name})
		MESSAGE(STATUS "IS SET ${qualified_plugin_name}")

		SET (PROTOCOL_DEFINITION_SOURCE_DIR ${CMAKE_SOURCE_DIR}/protocol-definitions)

		string(REPLACE "-" "_" tmp_plugin_name ${plugin})
		string(REPLACE "_plugin" "" tmp_plugin_name ${tmp_plugin_name})

		MESSAGE(STATUS "Converted Plugin Name::: ${file_name}")

		#which definition correspond to our plugin?
		FILE(GLOB PROTOCOL_DEFINITION_INPUT_FILES RELATIVE "${PROTOCOL_DEFINITION_SOURCE_DIR}"   "${PROTOCOL_DEFINITION_SOURCE_DIR}/target_agent_prot_${tmp_plugin_name}*.proto")			


		SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/plugins/${plugin}/gen)
		
		if (${PROTO_REMOVE_GEN_FILES})
            file (GLOB remove_file "${CMAKE_SOURCE_DIR}/plugins/${plugin}/gen/*.*")
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
		    COMMAND ${PROTO_COMMAND} --include_imports --proto_path=${PROTOCOL_DEFINITION_SOURCE_DIR} --cpp_out=${PROTOBUF_GEN_OUTPUT_DIR} ${PROTOCOL_DEFINITION_SOURCE_DIR}/${PROTOCOL_DEFINITION_INPUT_FILES}
		    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		)
		
		add_custom_target(proto_${plugin} ALL DEPENDS  ${PROTO_GEN_SOURCE_FILES} ${PROTO_GEN_HEADER_FILES})

		ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/plugins/${plugin})
		
		file (STRINGS "${CMAKE_SOURCE_DIR}/plugins/${plugin}/CMakeLists.txt" PLATFORMS)

		MESSAGE(STATUS "::::::::::::::::::::::::::::::::::::::::::::plugin ${plugin} COMPATIBILITY MATRIX ${${tmp_plugin_name}_platformSupportMatrix} ::::::::::::::::::::::::::::::::::::::::::::")
		MESSAGE(STATUS "::::::::::::::::::::::::::::::::::::::::::::plugin ${plugin} COMPATIBILITY MATRIX ${${tmp_plugin_name}_Capabilities} ::::::::::::::::::::::::::::::::::::::::::::")

		SET(IS_PLATFORM_SUPPORTED TRUE)
		IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "QNX")
			isConfigSupported( ${${tmp_plugin_name}_platformSupportMatrix} IS_PLATFORM_SUPPORTED)
		ENDIF()
		MESSAGE(STATUS ":::::::::::::::::::::::::::::::::::::::::::: ${IS_PLATFORM_SUPPORTED} ::::::::::::::::::::::::::::::::::::::::::::")

		if(${IS_PLATFORM_SUPPORTED} STREQUAL "FALSE")
		  MESSAGE(FATAL_ERROR "plugin ${plugin} cannot compile for the target platform [${CMAKE_SYSTEM_NAME},${CMAKE_SYSTEM_PROCESSOR}]")
		else()
		MESSAGE(STATUS "Compile plugin ${plugin}  for the following platform [${CMAKE_SYSTEM_NAME},${CMAKE_SYSTEM_PROCESSOR}]")
		endif()


		set(PLUGIN_CONFIG ${${tmp_plugin_name}_Capabilities})
	
    	set(PLUGIN_LIST "${PLUGIN_LIST}\n${PLUGIN_CONFIG}")
		
		unset(PROTOCOL_DEFINITION_INPUT_FILES)
		unset(PROTO_GEN_SOURCE_FILES)
		unset(PROTOBUF_GEN_OUTPUT_DIR)
		unset(PROTO_GEN_HEADER_FILES)
		unset(PLATFORMS)
		unset(TMP_VAR_NAME)
    else()
      MESSAGE(STATUS "IS NOT SET ${qualified_plugin_name}")
    endif()
ENDFOREACH()

set(PLUGINS_TO_LOAD ${PLUGIN_LIST} PARENT_SCOPE)

endfunction(load_plugins)


##exception------> until fix plugins  generated file
file (GLOB genFiles "${CMAKE_SOURCE_DIR}/plugins/most-spy-plugin/gen/*")
file (COPY ${genFiles} DESTINATION "${CMAKE_SOURCE_DIR}/plugins/dbus-monitor-plugin/gen")
file (COPY ${genFiles} DESTINATION "${CMAKE_SOURCE_DIR}/plugins/dlt-monitor-plugin/gen")
#####

load_plugins()

configure_file(${PROJECT_SOURCE_DIR}/utils/confTemplate.xml ${PROJECT_SOURCE_DIR}/utils/conf.xml )

### install
INSTALL(FILES  ${PROJECT_SOURCE_DIR}/utils/conf.xml DESTINATION ${CMAKE_INSTALL_PREFIX})


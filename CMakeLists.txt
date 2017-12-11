cmake_minimum_required(VERSION 2.8.5)

PROJECT ( solys-agent )

string(TIMESTAMP TS "%Y-%m-%d %H:%M:%S")
SET(TA_VERSION_INFO "Developer build from ${TS}" CACHE STRING "Target Agent Version related information")

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
	
	SET(${qualified_plugin_name} OFF CACHE BOOL "Build latest version of ${qualified_plugin_name}")
	
	MESSAGE(STATUS "Build latest version of ${qualified_plugin_name}")
	
	if(${qualified_plugin_name})
		MESSAGE(STATUS "IS SET ${qualified_plugin_name}")

		SET (PROTOCOL_DEFINITION_SOURCE_DIR ${CMAKE_SOURCE_DIR}/protocol-definitions)

		string(REPLACE "-" "_" tmp_plugin_name ${plugin})
		string(REPLACE "_plugin" "" tmp_plugin_name ${tmp_plugin_name})

		#MESSAGE(STATUS "Converted Plugin Name::: ${tmp_plugin_name}")

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
                    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --include_imports --proto_path=${PROTOCOL_DEFINITION_SOURCE_DIR} --cpp_out=${PROTOBUF_GEN_OUTPUT_DIR} ${PROTOCOL_DEFINITION_SOURCE_DIR}/${PROTOCOL_DEFINITION_INPUT_FILES}
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

      MESSAGE(STATUS "THE PLUGIN XML:: ${PLUGIN_LIST}")
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


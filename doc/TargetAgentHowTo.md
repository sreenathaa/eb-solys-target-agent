# EB solys Target Agent Usage Guide

## Content

	1. Dependencies
	2. Native Compilation
	3. Usage
	4. Cross Compilation
	5. Target Agent and plug-in configuration

##  1. Dependencies

Currently the compilation of EB solys Target Agent can only be done on a Linux system.

Following tools are needed:

	g++
	cmake
	autoconf
	automake
	libtool
	curl
	make
	git

The external libraries that the EB solys Target Agent (hereinafter called Target Agent) core uses are compiled and deployed together with the Target Agent itself.

Depending on the chosen plug-in configuration you should ensure that the plug-in dependencies are provided.

For example

	DLT_MONITOR_PLUGIN
		DLT Package:
		Version: 2.9.2

	DBUS_MONITOR
		D-Bus Message Bus Daemon
		Version  1.6.12

##  2.	Native Compilation

	export PATH_TO_SOURCES=”Target Agent source code”
	export PATH_TO_INSTALL=$PATH_TO_SOURCES/../install
	mkdir $PATH_TO_INSTALL
	mkdir $PATH_TO_SOURCES/../build
	cd $PATH_TO_SOURCES/../build

	cmake $PATH_TO_SOURCES -DTA_PLUGIN_dbus-monitor-plugin=true  -DTA_PLUGIN_socket-reader-plugin=true -DTA_PLUGIN_dlt-monitor-plugin=true -DTA_PLUGIN_resource-monitor-plugin=true -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX=$PATH_TO_INSTALL
	make all
	make install

## 3. Usage

Before starting the target agent you may want to adapt the existing static configuration as described in Chapter 5.

Start the target agent by calling following command:

	./solys-agent

Start the target agent with the --help parameter for information related to the accepted parameters and format

##  4. Cross Compilation

	export PATH_TO_SOURCES=”Target Agent source code”
	export PATH_TO_INSTALL=$PATH_TO_SOURCES/../install
	mkdir $PATH_TO_INSTALL
	mkdir $PATH_TO_SOURCES/../build
	cd $PATH_TO_SOURCES/../build
	export TOOLCHAIN=”path to cmake toolchain”

	cmake $PATH_TO_SOURCES -DTA_PLUGIN_dbus-monitor-plugin=true -DTA_PLUGIN_dlt-monitor-plugin=true -DTA_PLUGIN_resource-monitor-plugin=true -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX=$PATH_TO_INSTALL -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN

	make all
	make install

## 5. Target Agent and plug-in configuration

Target agent supports various configuration options. The performance footprint will vary with the selected configuration.
Below we provide a summary of the configuration options together with the associated impact.

## 5.1 Target Agent

### 5.1.1	Data Recording

By default the data collected by the target agent is written in file.
In case of a live connection data is also written to socket.
If you are only interested in live analysis or if you choose to save data on host side, then you may disable writing to file in order to improve performance.

For that purpose set the "enabled" option in the recorder XML element of config.xml:

	<Recorder>
		<!--Enable recording the communication to file -->
		<enabled>false</enabled>
	</Recorder>

Please note that target agent will continue to buffer the data received until the connection is established independent of the recorder configuration.

### 5.1.2	Internal Target Agent logging

Target agent offers several options for the output of the internal debug messages.
Performance may vary with the number of channels used for logging and the log level.
If you are not interested in the internal debug logs you may disable all of them by adapting the "ChannelActive" option in the config.xml

	<Logging>
		<Channels>
			<Console>
				<!--Enable writing target agent logs to console / Not Supported under WinCE -->
				<ChannelActive>false</ChannelActive>
			</Console>
			<File>
				<!--Enable writing target agent logs to console / Not Supported under WinCE -->
				<ChannelActive>false</ChannelActive>
			</File>
			<Socket>
				<!--Enable writing target agent logs to a TCP socket -->
				<ChannelActive>false</ChannelActive>
			</Socket>
		</Channels>
		<!-- Logging Level, one of:PRIO_FATAL (highest priority)/ PRIO_CRITICAL/ PRIO_ERROR/ PRIO_WARNING-->
		<!-- PRIO_NOTICE/ PRIO_INFORMATION/ PRIO_DEBUG/ PRIO_TRACE (lowest priority) -->
		<LoggingLevel>PRIO_ERROR</LoggingLevel>
	</Logging>

## 5.2 Target AgentPlug-ins:

### 5.2.1 Resource Monitor:

For gathering the resource utilization information the target agent polls the /proc file-system. The recommended sampling rate for resource usage data collection is one second.
Due to the fact that collecting this information involves exchanging data between kernel and user space,
user should be aware of the performance impact a lower resolution may have. The comment is equally valid for the amount of  processes considered for thread monitoring.

### 5.2.2 DLT Monitor

By default the DLT target agent plug-in logs the entire communication captured from the dlt-daemon.
If that is not the desired behaviour, then you may configure the DLT plug-in to gather only the logs coming from a specific application and/or having a specific context.

In the example below only the logs coming from the "APPLICATION_ID" application and having the "CONTEXT_ID" context will be monitored.

	<dlt-monitor-plugin>
		<dltFilter attr="APPLICATION_ID CONTEXT_ID"/>
	</dlt-monitor-plugin>

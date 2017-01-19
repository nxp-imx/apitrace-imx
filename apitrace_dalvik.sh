#! /system/bin/sh

# Copyright (c) 2014, Freescale Semiconductor Inc.
# All rights reserved.
#
# You should get a copy of license in this software package named EULA.txt.
# Please read EULA.txt carefully first.




# This script is used on Android to instrument non-native Android applications.
# For native Android applications, you can use "apitrace" as on Linux.
#
# Install this package under /data.
# The .trace files will be stored under /sdcard/.

# If install this package to other place, please update INSTALL in this file

if [ $# != 2 ] || [ "$2" != "start" -a "$2" != "stop" ]; then
	echo "$0 <component> <start|stop>"
	log -t apitrace "$0 <component> <start|stop>"
	exit 1
fi

PROCNAME=$1
INSTALL=/data/apitrace

if [ "$2" == "start" ]; then
	if [ -e $INSTALL/lib/apitrace/wrappers/egltrace.so ]; then
		# TODO: check normal user can access this path or not
		setprop wrap.$PROCNAME LD_PRELOAD=$INSTALL/lib/apitrace/wrappers/egltrace.so
	else
		# default
		setprop wrap.$PROCNAME LD_PRELOAD=/system/lib/egltrace.so
	fi

	setprop debug.apitrace.procname $PROCNAME

	log -t apitrace "Stop $PROCNAME/.main_dummy"
	am start -S $PROCNAME/.main_dummy
	sleep 5

	log -t apitrace "Start $PROCNAME"
	am start $PROCNAME
fi

if [ "$2" == "stop" ]; then
	log -t apitrace "Stop $PROCNAME tracing"
	log -t apitrace "Stop $PROCNAME/.main_dummy"
	am start -S $PROCNAME/.main_dummy
	setprop wrap.$PROCNAME ""
	setprop debug.apitrace.procname ""
fi

exit 0

#!/system/bin/sh

INSTALL=/data/apitrace
TRACEFILE_PATH=/sdcard/
MAXDURATION="10" # duration for every case. depending on available disk space

EGL11_CASES="angeles test-opengl-tritex test-opengl-textures"

kill_app()
{
    APP=$1
    # Need truncate app name, otherwise pkill will fail on yocto
    busybox pkill -x "${APP:0:15}"
}

run_eglcases()
{
    for tcase in $1
    do
        CURPATH="$(realpath .)"
        echo "--------------------------------------------------------"
	echo "Start apitrace"
        echo "--------------------------------------------------------"
        # start test
	apitrace trace -v --output=$TRACEFILE_PATH/$tcase.trace --api=egl $tcase &
        sleep $MAXDURATION
        # end test
	kill_app $tcase
        cd $CURPATH # go back to original path
    done
}

run_cases()
{
    export PATH=$PATH:$INSTALL/bin:$INSTALL/test

    echo "Put trace file to $TRACEFILE_PATH"

    mkdir -p $TRACEFILE_PATH
    rm $TRACEFILE_PATH/*.trace

    run_eglcases "$EGL11_CASES"
}

date

run_cases

echo "--------------------------------------------------------"
echo "Test finished"
date
echo "--------------------------------------------------------"
exit 0

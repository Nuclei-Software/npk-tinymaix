#!/bin/bash

LOGDIR=${LOGDIR:-logs/tinymaix}
CONFIG=${CONFIG:-n300}
DRYRUN=${DRYRUN:-0}

RUNYAML=req_runners.yaml

[ -f ${RUNYAML} ] && rm -f ${RUNYAML}
if [ "x$CONFIG" == "x" ] ; then
    echo "No config $CONFIG specified, exit"
    exit 1
fi

reqboards=""
if [[ "$CONFIG" == *"200"* ]] || [[ "$CONFIG" == *"300"* ]] ; then
    reqboards="ddr200t,${reqboards}"
fi
if [[ "$CONFIG" == *"600"* ]] ; then
    reqboards="ku060,${reqboards}"
fi
if [[ "$CONFIG" == *"900"* ]] ; then
    reqboards="vcu118,${reqboards}"
fi

runboard --request ${reqboards}

if [ -f ${RUNYAML} ] ; then
    echo "Requested required boards ${reqboards}"
else
    echo "No boards requested!"
    exit 1
fi

runcmd="python3 $NUCLEI_SDK_ROOT/tools/scripts/nsdk_cli/nsdk_runner.py --appyaml configs/tinymaix.yaml --runyaml ${RUNYAML} --logdir ${LOGDIR} --runon fpga --config ${CONFIG}"

echo $runcmd
if [ "x$DRYRUN" == "x0" ] ; then
    if [ "x${NUCLEI_SDK_ROOT}" == "x" ] ; then
        echo "NUCLEI_SDK_ROOT is not set in environment variables"
        exit 1
    fi
    eval $runcmd
fi

runboard --release ${RUNYAML}

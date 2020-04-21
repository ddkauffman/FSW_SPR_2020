#!/bin/sh

export CFS_MISSION="/home/fsw/cfs"

echo "Using ${CFS_MISSION} as CFS root ..."

# copy project source into cfs, build and run.

set -x && yes | cp -rf ./pm ${CFS_MISSION}/apps \
    && yes | cp -rf ./sch/fsw/tables/ ${CFS_MISSION}/apps/sch/fsw/ \
    && yes | cp -rf workbooks/* ~/cfs_demo/config/targets/CFS/cmd_tlm/ \
    && yes | cp ./supplimental/to_lab_sub_table.h ${CFS_MISSION}/build/cpu1/inc/to_lab_sub_table.h \
    && echo "Finished copying files! starting build ..." \
    && cd ${CFS_MISSION} \
    && . ./setvars.sh \
    && cd ${CFS_MISSION}/build/cpu1 && make clean && make config && make \
    && cd ${CFS_MISSION}/build/cpu1/exe \
    && sudo ./core-linux.bin --reset=PO;

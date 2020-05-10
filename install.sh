#!/bin/sh

export CFS_MISSION="/home/fsw/cfs"

echo "INFO: Using ${CFS_MISSION} as CFS root ..."
echo "INFO: Copying  Power Management Application ..."
# copy project source into cfs, build and run.

set -x && yes | cp -rf ./pm ${CFS_MISSION}/apps \
    && yes | cp -rf ./workbooks/* ~/cfs_demo/config/targets/CFS/cmd_tlm/ \
    && yes | cp -rf ./screens/wise.txt ~/cfs_demo/config/targets/CFS/screens/wise.txt \
    && yes | cp -rf ./viewer/tlm_viewer.txt ~/cfs_demo/config/tools/tlm_viewer/tlm_viewer.txt \
    && yes | cp ./supplimental/to_lab_sub_table.h ${CFS_MISSION}/build/cpu1/inc/to_lab_sub_table.h \
    && echo "INFO: Copying WISE instrument files ..." \
    && yes | cp -rf ./supplimental/wise ${CFS_MISSION}/apps \
    && yes | cp -rf ./supplimental/wise/fsw/platform_inc/* ${CFS_MISSION}/build/cpu1/inc \
    && echo "INFO: Copying tables and build config ..." \
    && yes | cp -rf ./tables/* ${CFS_MISSION}/apps/sch/fsw/tables \
    && yes | cp -f ./config/Makefile ${CFS_MISSION}/build/cpu1/Makefile \
    && yes | cp -f ./config/cfe_es_startup.scr ${CFS_MISSION}/build/cpu1/exe/cfe_es_startup.scr \
    && echo "Finished copying files! starting build ..." \
    && cd ${CFS_MISSION} \
    && . ./setvars.sh \
    && cd ${CFS_MISSION}/build/cpu1 && make clean && make config && make \
    && cd ${CFS_MISSION}/build/cpu1/exe \
    && sudo ./core-linux.bin --reset=PO;

#!/bin/sh

export CFS_MISSION="/home/fsw/cfs"

echo "Using ${CFS_MISSION} as CFS root ..."

# Overwrite the PM in the cfs with the repo version
set -x && yes | cp -rf ./pm ${CFS_MISSION}/apps

# Overwrite the sch in the cfs with the repo version
set -x && yes | cp -rf ./sch/fsw/tables/ ${CFS_MISSION}/apps/sch/fsw/

# Overwrite project workbooks into the config folder
set -x && yes | cp -rf workbooks/* ~/cfs_demo/config/targets/CFS/cmd_tlm/

echo "Finished copying files! starting build ..."

cd ${CFS_MISSION}

set -x && . ./setvars.sh

set -x && cd ${CFS_MISSION}/build/cpu1 && make clean && make;

set -x && cd ${CFS_MISSION}/build/cpu1/exe && ./core-linux.bin --reset=PO;



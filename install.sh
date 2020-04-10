#!/bin/sh

export CFS_MISSION=~/cfs

# Overwrite the PM in the cfs with the repo version
set -x && yes | cp -rf pm ${CFS_MISSION}/apps;

# Overwrite project workbooks into the config folder
set -x && yes | cp -rf workbooks/* ~/cfs_demo/config/targets/CFS/cmd_tlm/

echo "Done!"
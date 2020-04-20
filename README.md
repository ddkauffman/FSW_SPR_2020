# Power Management Flight Software

This is a student project as part of the Space Systems Engineering for Engineering Professionals
at Johns Hopkins University. The purpose of the application is to determine whether the power
distribution unit within the instrument assembly has appropriete capacitor charge to perform
an observation.

# Setup

Add `CFE_APP, /cf/apps/pm.so,  PM_AppMain, PM,  100, 8192,  0x0, 0;`  to `${CFS_MISSION}` /home/fsw/cfs/build/cpu1/exe/cfe_es_startup.scr

Add `THE_APPS += pm` to `${CFS_MISSION}cfs/build/cpu1/Makefile`

# Install

```bash
cd
git clone https://github.com/ddkauffman/FSW_SPR_2020.git fsw_spr_2020
cd fsw_spr_2020
sudo ./install.sh
```
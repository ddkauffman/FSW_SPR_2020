/*=======================================================================================
** File Name:  pm_app.h
**
** Title:  Header File for PM Application
**
** $Author:    fsw
** $Revision: 1.1 $
** $Date:      2020-04-09
**
** Purpose:  To define PM's internal macros, data types, global variables and
**           function prototypes
**
** Modification History:
**   Date | Author | Description
**   ---------------------------
**   2020-04-09 | fsw | Build #: Code Started
**
**=====================================================================================*/
    
#ifndef _PM_APP_H_
#define _PM_APP_H_

/*
** Pragmas
*/

/*
** Include Files
*/
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "pm_platform_cfg.h"
#include "pm_mission_cfg.h"
#include "pm_private_ids.h"
#include "pm_private_types.h"
#include "pm_perfids.h"
#include "pm_msgids.h"
#include "pm_msg.h"



/*
** Local Defines
*/
#define PM_TIMEOUT_MSEC    1000

/*
** Local Structure Declarations
*/
typedef struct
{
    /* CFE Event table */
    CFE_EVS_BinFilter_t  EventTbl[PM_EVT_CNT];

    /* CFE scheduling pipe */
    CFE_SB_PipeId_t  SchPipeId; 
    uint16           usSchPipeDepth;
    char             cSchPipeName[OS_MAX_API_NAME];

    /* CFE command pipe */
    CFE_SB_PipeId_t  CmdPipeId;
    uint16           usCmdPipeDepth;
    char             cCmdPipeName[OS_MAX_API_NAME];
    
    /* CFE telemetry pipe */
    CFE_SB_PipeId_t  TlmPipeId;
    uint16           usTlmPipeDepth;
    char             cTlmPipeName[OS_MAX_API_NAME];

    /* Task-related */
    uint32  uiRunStatus;
    
    /* Input data - from I/O devices or subscribed from other apps' output data.
       Data structure should be defined in pm/fsw/src/pm_private_types.h */
    PM_InData_t   InData;

    /* Output data - to be published at the end of a Wakeup cycle.
       Data structure should be defined in pm/fsw/src/pm_private_types.h */
    PM_OutData_t  OutData;

    /* Housekeeping telemetry - for downlink only.
       Data structure should be defined in pm/fsw/src/pm_msg.h */
    PM_HkTlm_t  HkTlm;

    /* TODO:  Add declarations for additional private data here */
} PM_AppData_t;

/*
** External Global Variables
*/

/*
** Global Variables
*/

/*
** Local Variables
*/

/*
** Local Function Prototypes
*/
int32  PM_InitApp(void);
int32  PM_InitEvent(void);
int32  PM_InitData(void);
int32  PM_InitPipe(void);

void  PM_AppMain(void);

void  PM_CleanupCallback(void);

int32  PM_RcvMsg(int32 iBlocking);

void  PM_ProcessNewData(void);
void  PM_ProcessNewCmds(void);
void  PM_ProcessNewAppCmds(CFE_SB_Msg_t*);

void  PM_ReportHousekeeping(void);
void  PM_SendOutData(void);

boolean  PM_VerifyCmdLength(CFE_SB_Msg_t*, uint16);

#endif /* _PM_APP_H_ */

/*=======================================================================================
** End of file pm_app.h
**=====================================================================================*/
    
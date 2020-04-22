/*=======================================================================================
** File Name:  pm_msg.h
**
** Title:  Message Definition Header File for PM Application
**
** $Author:    fsw
** $Revision: 1.1 $
** $Date:      2020-04-09
**
** Purpose:  To define PM's command and telemetry message defintions 
**
** Modification History:
**   Date | Author | Description
**   ---------------------------
**   2020-04-09 | fsw | Build #: Code Started
**
**=====================================================================================*/
    
#ifndef _PM_MSG_H_
#define _PM_MSG_H_

/*
** Pragmas
*/

/*
** Include Files
*/



/*
** Local Defines
*/

/*
** PM command codes
*/
#define PM_NOOP_CC                 0
#define PM_RESET_CC                1
#define PM_TO_WISE_CAP_ACTIVE_CC   2
#define PM_TO_WISE_CAP_DISCHARGE_CC 3
#define PM_TO_WISE_POWER_SBC_CC 4

/*
** Local Structure Declarations
*/
typedef struct
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              usCmdCnt;
    uint8              usCmdErrCnt;
    uint8              actCap;

    /* TODO:  Add declarations for additional housekeeping data here */

} PM_HkTlm_t;

typedef struct
{
    uint8              CmdHeader[CFE_SB_CMD_HDR_SIZE];

    uint8              setCap;

    /* TODO:  Add declarations for additional housekeeping data here */

} PM_TO_WISE_CAP_ACTIVE_t;
#endif /* _PM_MSG_H_ */

/*=======================================================================================
** End of file pm_msg.h
**=====================================================================================*/
    

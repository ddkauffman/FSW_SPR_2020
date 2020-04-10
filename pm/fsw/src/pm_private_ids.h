/*=======================================================================================
** File Name:  pm_private_ids.h
**
** Title:  ID Header File for PM Application
**
** $Author:    fsw
** $Revision: 1.1 $
** $Date:      2020-04-09
**
** Purpose:  This header file contains declarations and definitions of PM's private IDs.
**
** Modification History:
**   Date | Author | Description
**   ---------------------------
**   2020-04-09 | fsw | Build #: Code Started
**
**=====================================================================================*/
    
#ifndef _PM_PRIVATE_IDS_H_
#define _PM_PRIVATE_IDS_H_

/*
** Pragmas
*/

/*
** Include Files
*/

/*
** Local Defines
*/

/* Event IDs */
#define PM_RESERVED_EID  0

#define PM_INF_EID        1
#define PM_INIT_INF_EID   2
#define PM_ILOAD_INF_EID  3
#define PM_CDS_INF_EID    4
#define PM_CMD_INF_EID    5

#define PM_ERR_EID         51
#define PM_INIT_ERR_EID    52
#define PM_ILOAD_ERR_EID   53
#define PM_CDS_ERR_EID     54
#define PM_CMD_ERR_EID     55
#define PM_PIPE_ERR_EID    56
#define PM_MSGID_ERR_EID   57
#define PM_MSGLEN_ERR_EID  58

#define PM_EVT_CNT  14

/*
** Local Structure Declarations
*/

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

#endif /* _PM_PRIVATE_IDS_H_ */

/*=======================================================================================
** End of file pm_private_ids.h
**=====================================================================================*/
    
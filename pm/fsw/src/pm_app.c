/*=======================================================================================
** File Name:  pm_app.c
**
** Title:  Function Definitions for PM Application
**
** $Author:    fsw
** $Revision: 1.1 $
** $Date:      2020-04-09
**
** Purpose:  This source file contains all necessary function definitions to run PM
**           application.
**
** Functions Defined:
**    Function X - Brief purpose of function X
**    Function Y - Brief purpose of function Y
**    Function Z - Brief purpose of function Z
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to all functions in the file.
**    2. List the external source(s) and event(s) that can cause the funcs in this
**       file to execute.
**    3. List known limitations that apply to the funcs in this file.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Modification History:
**   Date | Author | Description
**   ---------------------------
**   2020-04-09 | fsw | Build #: Code Started
**
**=====================================================================================*/

/*
** Pragmas
*/

/*
** Include Files
*/
#include <string.h>

#include "cfe.h"

#include "pm_platform_cfg.h"
#include "pm_mission_cfg.h"
#include "pm_app.h"
#include "/home/fsw/cfs/apps/wise/fsw/src/wise_msg.h"
#include "/home/fsw/cfs/apps/wise/fsw/platform_inc/wise_msgids.h"


// include msgids from whe
// #include "whe_msgids.h"

/*
** Local Defines
*/
#define MAX_SAFE_CHARGE 10000 // 100%
#define MIN_SAFE_CHARGE 0 // 0%
#define MIN_SAFE_DURING_OBS 500 // -5%

#define SECONDS_TO_START_DISCHARGE 300
#define SECONDS_TO_COMPLETE_OBS 10
#define PERCENT_DISCHARGE_PER_SECOND 10 

#define MAX_CAP_LOSE_PER_SECOND 700 // 7%

#define ALL_CAPS_CHARGE_RATE 100 // 3%
#define TWO_CAPS_CHARGE_RATE 150 // 1.5%
#define ONE_CAPS_CHARGE_RATE 300 // 1%

#define MAX_CHARGE 6000
#define MIN_OBS_CHARGE 1000

#define CAP_A 0
#define CAP_B 1
#define CAP_C 2

#define CHARGING 0
#define LEAKING 1
#define DISCHARGING 2
#define BROKEN 3

#define OBSERVING 2

/*
** Local Structure Declarations
*/

/*
** External Global Variables
*/

/*
** Global Variables
*/
PM_AppData_t  g_PM_AppData;
WISE_HkTlm_t* g_WISE_HkTlm;

uint16 cap_charges[3];
uint16 cap_states[3];
uint16 caps_time_to_charge[3];

uint16 gActiveCapcitor;
uint16 gWiseSbcState;



/*
** Local Variables
*/
int cap_pick;

/*
** Local Function Definitions
*/


int select_max_cap(int capacitor){
    
    int selected_capacitor = NULL;
        
    switch(capacitor){
        case CAP_A:;
            selected_capacitor = max_of_two_charge(max_of_two_charge(CAP_A, CAP_B), max_of_two_charge(CAP_A, CAP_C));
            break;
        case CAP_B:;
            selected_capacitor = max_of_two_charge(max_of_two_charge(CAP_B, CAP_A), max_of_two_charge(CAP_B, CAP_C));
            break;
        case CAP_C:;
            selected_capacitor = max_of_two_charge(max_of_two_charge(CAP_C, CAP_A), max_of_two_charge(CAP_C, CAP_B));
            break;
    } 
        
    return selected_capacitor;
} 

int select_min_cap(int capacitor){
    
    int selected_capacitor = NULL;
        
    switch(capacitor){
        case CAP_A:;
            selected_capacitor = min_of_two_charge(min_of_two_charge(CAP_A, CAP_B), min_of_two_charge(CAP_A, CAP_C));
            break;
        case CAP_B:;
            selected_capacitor = min_of_two_charge(min_of_two_charge(CAP_B, CAP_A), min_of_two_charge(CAP_B, CAP_C));
            break;
        case CAP_C:;
            selected_capacitor = min_of_two_charge(min_of_two_charge(CAP_C, CAP_A), min_of_two_charge(CAP_C, CAP_B));
            break;
    } 
        
    return selected_capacitor;
}  

int select_active_capacitor(){

    int selected_capacitor = NULL;

    if(g_WISE_HkTlm->wiseSbcState == OBSERVING && cap_charges[g_WISE_HkTlm->wiseActiveCap] <= MIN_OBS_CHARGE + 700){
            int highest = 0;
            int idx = 0;

            for(int i = 0; i < 3; i++){
                if(cap_charges[i] > highest){
                    highest = cap_charges[i];
                    idx = i; 
                }
                
            }
            set_wise_active_cap(idx);
            return NULL;
    }

    for(int capacitor = 0; capacitor < 3; capacitor++){

        

        if(cap_charges[capacitor] < MIN_OBS_CHARGE){
            continue;
        }

        if(cap_states == LEAKING){
            continue;
        }

        if(cap_states == BROKEN){
            continue;
        }

       

        if(capacitor == g_WISE_HkTlm->wiseActiveCap && cap_charges[capacitor] >= MAX_SAFE_CHARGE - 500){
            selected_capacitor = select_min_cap(capacitor);
            return selected_capacitor;
        }
        
        if(cap_charges[capacitor] >= MAX_SAFE_CHARGE - (SECONDS_TO_START_DISCHARGE)){
            selected_capacitor = select_max_cap(capacitor);
            return selected_capacitor;
        }   
    }

    return selected_capacitor;
}

int min_of_two_charge(int CAP_1,  int CAP_2){
    if(cap_charges[CAP_1] < cap_charges[CAP_2]){
        return CAP_1;
    } 
    return CAP_2;
}

int max_of_two_charge(int CAP_1,  int CAP_2){
    return (cap_charges[CAP_1] > cap_charges[CAP_2]) ? CAP_1 : CAP_2;
}

void send_discharge_command(uint8 cap){
    // If we aren't observing with this cap send discharge

    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                        "PM - Thinking abuot sending dis for cap (%d)", cap);

    /* define command to send */
    WISE_ParmCmd_t cmd;
    memset((void*)&cmd, 0x00, sizeof(WISE_ParmCmd_t));

    /* Initialize message to send to WISE*/
    CFE_SB_InitMsg(&cmd, WISE_CMD_MID, sizeof(cmd), TRUE);

    /* Set command code for message */
    CFE_SB_SetCmdCode((WISE_ParmCmd_t*)&cmd, WISE_CAP_DISCHARGE_CC);
    cmd.target = cap; 

    /* Set Timestamp of message */
    CFE_SB_TimeStampMsg((WISE_ParmCmd_t*)&cmd);

    /* Send the message on the CFE Bus */
    CFE_SB_SendMsg((WISE_ParmCmd_t*)&cmd);

    return 0;
}

void set_wise_active_cap(uint8 cap){

    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                        "PM - Thinking abuot sending active for cap (%d)", cap);

        /* define command to send */
        WISE_ParmCmd_t cmd;
        memset((void*)&cmd, 0x00, sizeof(WISE_ParmCmd_t));

        /* Initialize message to send to whe*/
        CFE_SB_InitMsg(&cmd, WISE_CMD_MID, sizeof(cmd), TRUE);

        /* Set command code for message */
        CFE_SB_SetCmdCode((WISE_ParmCmd_t*)&cmd, WISE_CAP_ACTIVE_CC);
        cmd.target = cap;

        /* Set Timestamp of message */
        CFE_SB_TimeStampMsg((WISE_ParmCmd_t*)&cmd);

        /* Send the message on the CFE Bus */
        CFE_SB_SendMsg((WISE_ParmCmd_t*)&cmd);
}

int get_charge_rate(int num_failing_caps){
    switch(num_failing_caps){
        case 0:
            return ALL_CAPS_CHARGE_RATE;
            break;
        case 1:
            return TWO_CAPS_CHARGE_RATE;
            break;
        case 2:
            return ONE_CAPS_CHARGE_RATE;
            break;
        case 3:
            return 0;
            break;
    }

    return -1;
}

int get_number_of_failing_caps(){
    int num_of_failing_caps = 0;

    for(int i = 0; i < 3; i++){
        if(cap_states[i] == LEAKING || cap_states[i] == BROKEN){
            ++num_of_failing_caps;
        } 
    }

    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                        "PM - There are {%d} failing caps!!!!!!!!!1!!!", num_of_failing_caps);

    return num_of_failing_caps;
}

int get_number_of_discharging_caps(uint16 caps[]){
    int number_of_caps_discharging = 0;

    for(int i = 0; i < 3; i++){
        if(caps[i] == 2){
            ++number_of_caps_discharging;
        } 
    }

    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                        "PM - Getting Number of Dis Caps {%d}", number_of_caps_discharging);

    return number_of_caps_discharging;
}

void calculate_time_to_charge(){
    for(int i = 0; i < 3; i++){

        CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                        "PM - STEP 1 (%d)", (10000 - cap_charges[i]));

        CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                        "PM - STEP 2 (%d)", get_charge_rate(get_number_of_failing_caps()));

        caps_time_to_charge[i] = (10000 - cap_charges[i])/get_charge_rate(get_number_of_failing_caps());
    }
}

void capacitor_charge_handler(int buffer){

    int number_of_caps_discharging = get_number_of_discharging_caps(cap_states);

    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                        "PM - Number of discharging CAPS!!!! (%d)", number_of_caps_discharging);
    
    if(number_of_caps_discharging > 1){
        CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                            "PM - Max Capacitors Being Discharged"); 
        return;
    } else {
       for(int capacitor = 0; capacitor < 3; capacitor++){


            if(capacitor == g_WISE_HkTlm->wiseActiveCap){
                continue;
            }

            if(cap_charges[capacitor] >= MAX_SAFE_CHARGE){
               send_discharge_command(capacitor);
               return;   
            } 


           if(cap_charges[capacitor] >= MAX_SAFE_CHARGE - (SECONDS_TO_START_DISCHARGE + (cap_charges[capacitor] * .75)) || caps_time_to_charge[capacitor] < 5){
                if(cap_states[capacitor] == DISCHARGING){
                    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                    "PM - CAP Doesn't need to be discharged");
                    continue;
                }

                
                if(capacitor == g_WISE_HkTlm->wiseActiveCap && g_WISE_HkTlm->wiseSbcState == OBSERVING){
                    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                            "PM - Active Cap should already discharge during OBS");
                    continue; 
                } 
                if(cap_charges[capacitor] < 5500){
                    continue;
                }
                send_discharge_command(capacitor); 
           }
       } 
    }
}

void PM_ProcessWISEData(CFE_SB_Msg_t* TlmMsgPtr){
    
    // Global Reference to WISE HK TLM
    g_WISE_HkTlm = (WISE_HkTlm_t *) TlmMsgPtr;

    cap_charges[0] = g_WISE_HkTlm->wiseCapA_Charge;
    cap_charges[1] = g_WISE_HkTlm->wiseCapB_Charge;
    cap_charges[2] = g_WISE_HkTlm->wiseCapC_Charge;
  
    cap_states[0] = g_WISE_HkTlm->wiseCapA_State;
    cap_states[1] = g_WISE_HkTlm->wiseCapB_State,
    cap_states[2] = g_WISE_HkTlm->wiseCapC_State; 

    calculate_time_to_charge();

    switch(g_WISE_HkTlm->wiseSbcState){
        case 0:
            CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                "PM - WISE SBC STATE: (%d), SBC Off, nothing to manage.", g_WISE_HkTlm->wiseSbcState);
            break;
        case 1:;
            capacitor_charge_handler(5000);
            cap_pick = select_active_capacitor();
            if(cap_pick != NULL){
                set_wise_active_cap(cap_pick);
            }
            break;
        case 2:;
            capacitor_charge_handler(0);
            cap_pick = select_active_capacitor();
            if(cap_pick != NULL){
                set_wise_active_cap(cap_pick);
            }
            break; 
        case 3:;
             CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                "PM - WISE SBC STATE: (%d), SBC ERROR!!! Cannot take further action!", g_WISE_HkTlm->wiseSbcState);
            break;
    }
}
    
/*=====================================================================================
** Name: PM_InitEvent
**
** Purpose: To initialize and register event table for PM application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_EVS_Register
**    CFE_ES_WriteToSysLog
**
** Called By:
**    PM_InitApp
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    g_PM_AppData.EventTbl
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 PM_InitEvent()
{
    int32  iStatus=CFE_SUCCESS;

    /* Create the event table */
    memset((void*)g_PM_AppData.EventTbl, 0x00, sizeof(g_PM_AppData.EventTbl));

    g_PM_AppData.EventTbl[0].EventID = PM_RESERVED_EID;
    g_PM_AppData.EventTbl[1].EventID = PM_INF_EID;
    g_PM_AppData.EventTbl[2].EventID = PM_INIT_INF_EID;
    g_PM_AppData.EventTbl[3].EventID = PM_ILOAD_INF_EID;
    g_PM_AppData.EventTbl[4].EventID = PM_CDS_INF_EID;
    g_PM_AppData.EventTbl[5].EventID = PM_CMD_INF_EID;

    g_PM_AppData.EventTbl[ 6].EventID = PM_ERR_EID;
    g_PM_AppData.EventTbl[ 7].EventID = PM_INIT_ERR_EID;
    g_PM_AppData.EventTbl[ 8].EventID = PM_ILOAD_ERR_EID;
    g_PM_AppData.EventTbl[ 9].EventID = PM_CDS_ERR_EID;
    g_PM_AppData.EventTbl[10].EventID = PM_CMD_ERR_EID;
    g_PM_AppData.EventTbl[11].EventID = PM_PIPE_ERR_EID;
    g_PM_AppData.EventTbl[12].EventID = PM_MSGID_ERR_EID;
    g_PM_AppData.EventTbl[13].EventID = PM_MSGLEN_ERR_EID;

    /* Register the table with CFE */
    iStatus = CFE_EVS_Register(g_PM_AppData.EventTbl,
                               PM_EVT_CNT, CFE_EVS_BINARY_FILTER);
    if (iStatus != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PM - Failed to register with EVS (0x%08X)\n", iStatus);
    }

    return (iStatus);
}
    
/*=====================================================================================
** Name: PM_InitPipe
**
** Purpose: To initialize all message pipes and subscribe to messages for PM application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_SB_CreatePipe
**    CFE_SB_Subscribe
**    CFE_ES_WriteToSysLog
**
** Called By:
**    PM_InitApp
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    g_PM_AppData.usSchPipeDepth
**    g_PM_AppData.cSchPipeName
**    g_PM_AppData.SchPipeId
**    g_PM_AppData.usCmdPipeDepth
**    g_PM_AppData.cCmdPipeName
**    g_PM_AppData.CmdPipeId
**    g_PM_AppData.usTlmPipeDepth
**    g_PM_AppData.cTlmPipeName
**    g_PM_AppData.TlmPipeId
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 PM_InitPipe()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init schedule pipe */
    g_PM_AppData.usSchPipeDepth = PM_SCH_PIPE_DEPTH;
    memset((void*)g_PM_AppData.cSchPipeName, '\0', sizeof(g_PM_AppData.cSchPipeName));
    strncpy(g_PM_AppData.cSchPipeName, "PM_SCH_PIPE", OS_MAX_API_NAME-1);

    /* Subscribe to Wakeup messages */
    iStatus = CFE_SB_CreatePipe(&g_PM_AppData.SchPipeId,
                                 g_PM_AppData.usSchPipeDepth,
                                 g_PM_AppData.cSchPipeName);
    if (iStatus == CFE_SUCCESS)
    {
        iStatus = CFE_SB_SubscribeEx(PM_WAKEUP_MID, g_PM_AppData.SchPipeId, CFE_SB_Default_Qos, 1);

        if (iStatus != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("PM - Sch Pipe failed to subscribe to PM_WAKEUP_MID. (0x%08X)\n", iStatus);
            goto PM_InitPipe_Exit_Tag;
        }
        
    }
    else
    {
        CFE_ES_WriteToSysLog("PM - Failed to create SCH pipe (0x%08X)\n", iStatus);
        goto PM_InitPipe_Exit_Tag;
    }

    /* Init command pipe */
    g_PM_AppData.usCmdPipeDepth = PM_CMD_PIPE_DEPTH ;
    memset((void*)g_PM_AppData.cCmdPipeName, '\0', sizeof(g_PM_AppData.cCmdPipeName));
    strncpy(g_PM_AppData.cCmdPipeName, "PM_CMD_PIPE", OS_MAX_API_NAME-1);

    /* Subscribe to command messages */
    iStatus = CFE_SB_CreatePipe(&g_PM_AppData.CmdPipeId,
                                 g_PM_AppData.usCmdPipeDepth,
                                 g_PM_AppData.cCmdPipeName);
    if (iStatus == CFE_SUCCESS)
    {
        /* Subscribe to command messages */
        iStatus = CFE_SB_Subscribe(PM_CMD_MID, g_PM_AppData.CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("PM - CMD Pipe failed to subscribe to PM_CMD_MID. (0x%08X)\n", iStatus);
            goto PM_InitPipe_Exit_Tag;
        }

        iStatus = CFE_SB_Subscribe(PM_SEND_HK_MID, g_PM_AppData.CmdPipeId);

        if (iStatus != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("PM - CMD Pipe failed to subscribe to PM_SEND_HK_MID. (0x%08X)\n", iStatus);
            goto PM_InitPipe_Exit_Tag;
        }
        
    }
    else
    {
        CFE_ES_WriteToSysLog("PM - Failed to create CMD pipe (0x%08X)\n", iStatus);
        goto PM_InitPipe_Exit_Tag;
    }

    /* Init telemetry pipe */
    g_PM_AppData.usTlmPipeDepth = PM_TLM_PIPE_DEPTH;
    memset((void*)g_PM_AppData.cTlmPipeName, '\0', sizeof(g_PM_AppData.cTlmPipeName));
    strncpy(g_PM_AppData.cTlmPipeName, "PM_TLM_PIPE", OS_MAX_API_NAME-1);

    /* Subscribe to telemetry messages on the telemetry pipe */
    iStatus = CFE_SB_CreatePipe(&g_PM_AppData.TlmPipeId,
                                 g_PM_AppData.usTlmPipeDepth,
                                 g_PM_AppData.cTlmPipeName);
    if (iStatus == CFE_SUCCESS)
    {
        /* TODO:  Add CFE_SB_Subscribe() calls for other apps' output data here.
        **
        ** Examples:
        **     CFE_SB_Subscribe(GNCEXEC_OUT_DATA_MID, g_PM_AppData.TlmPipeId);
        */
        
        // Subscribe to the WISE Instrument HK TLM 
        CFE_SB_Subscribe(WISE_HK_TLM_MID, g_PM_AppData.TlmPipeId);
    }
    else
    {
        CFE_ES_WriteToSysLog("PM - Failed to create TLM pipe (0x%08X)\n", iStatus);
        goto PM_InitPipe_Exit_Tag;
    }

PM_InitPipe_Exit_Tag:
    return (iStatus);
}
    
/*=====================================================================================
** Name: PM_InitData
**
** Purpose: To initialize global variables used by PM application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_SB_InitMsg
**
** Called By:
**    PM_InitApp
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    g_PM_AppData.InData
**    g_PM_AppData.OutData
**    g_PM_AppData.HkTlm
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 PM_InitData()
{
    int32  iStatus=CFE_SUCCESS;

    /* Init input data */
    memset((void*)&g_PM_AppData.InData, 0x00, sizeof(g_PM_AppData.InData));

    /* Init output data */
    memset((void*)&g_PM_AppData.OutData, 0x00, sizeof(g_PM_AppData.OutData));
    CFE_SB_InitMsg(&g_PM_AppData.OutData,
                   PM_OUT_DATA_MID, sizeof(g_PM_AppData.OutData), TRUE);

    /* Init housekeeping packet */
    memset((void*)&g_PM_AppData.HkTlm, 0x00, sizeof(g_PM_AppData.HkTlm));
    CFE_SB_InitMsg(&g_PM_AppData.HkTlm,
                   PM_HK_TLM_MID, sizeof(g_PM_AppData.HkTlm), TRUE);

    return (iStatus);
}
    
/*=====================================================================================
** Name: PM_InitApp
**
** Purpose: To initialize all data local to and used by PM application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization
**
** Routines Called:
**    CFE_ES_RegisterApp
**    CFE_ES_WriteToSysLog
**    CFE_EVS_SendEvent
**    OS_TaskInstallDeleteHandler
**    PM_InitEvent
**    PM_InitPipe
**    PM_InitData
**
** Called By:
**    PM_AppMain
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 PM_InitApp()
{
    int32  iStatus=CFE_SUCCESS;

    g_PM_AppData.uiRunStatus = CFE_ES_APP_RUN;

    iStatus = CFE_ES_RegisterApp();
    if (iStatus != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PM - Failed to register the app (0x%08X)\n", iStatus);
        goto PM_InitApp_Exit_Tag;
    }

    if ((PM_InitEvent() != CFE_SUCCESS) || 
        (PM_InitPipe() != CFE_SUCCESS) || 
        (PM_InitData() != CFE_SUCCESS))
    {
        iStatus = -1;
        goto PM_InitApp_Exit_Tag;
    }

    /* Install the cleanup callback */
    OS_TaskInstallDeleteHandler((void*)&PM_CleanupCallback);

PM_InitApp_Exit_Tag:
    if (iStatus == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PM_INIT_INF_EID, CFE_EVS_INFORMATION,
                          "PM - Application initialized");
    }
    else
    {
        CFE_ES_WriteToSysLog("PM - Application failed to initialize\n");
    }

    return (iStatus);
}
    
/*=====================================================================================
** Name: PM_CleanupCallback
**
** Purpose: To handle any neccesary cleanup prior to application exit
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    TBD
**
** Called By:
**    TBD
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void PM_CleanupCallback()
{
    /* TODO:  Add code to cleanup memory and other cleanup here */
}
    
/*=====================================================================================
** Name: PM_RcvMsg
**
** Purpose: To receive and process messages for PM application
**
** Arguments:
**    None
**
** Returns:
**    int32 iStatus - Status of initialization 
**
** Routines Called:
**    CFE_SB_RcvMsg
**    CFE_SB_GetMsgId
**    CFE_EVS_SendEvent
**    CFE_ES_PerfLogEntry
**    CFE_ES_PerfLogExit
**    PM_ProcessNewCmds
**    PM_ProcessNewData
**    PM_SendOutData
**
** Called By:
**    PM_Main
**
** Global Inputs/Reads:
**    g_PM_AppData.SchPipeId
**
** Global Outputs/Writes:
**    g_PM_AppData.uiRunStatus
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
int32 PM_RcvMsg(int32 iBlocking)
{
    int32           iStatus=CFE_SUCCESS;
    CFE_SB_Msg_t*   MsgPtr=NULL;
    CFE_SB_MsgId_t  MsgId;

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(PM_MAIN_TASK_PERF_ID);

    /* Wait for WakeUp messages from scheduler */
    iStatus = CFE_SB_RcvMsg(&MsgPtr, g_PM_AppData.SchPipeId, iBlocking);

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(PM_MAIN_TASK_PERF_ID);

    if (iStatus == CFE_SUCCESS)
    {
        MsgId = CFE_SB_GetMsgId(MsgPtr);
        switch (MsgId)
	{
            case PM_WAKEUP_MID:
                PM_ProcessNewCmds();
                PM_ProcessNewData();

                /* TODO:  Add more code here to handle other things when app wakes up */

                /* The last thing to do at the end of this Wakeup cycle should be to
                   automatically publish new output. */
                PM_SendOutData();
                break;

            default:
                CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "PM - Recvd invalid SCH msgId (0x%08X)", MsgId);
        }
    }
    else if (iStatus == CFE_SB_NO_MESSAGE)
    {
        /* If there's no incoming message, you can do something here, or nothing */
    }
    else
    {
        /* This is an example of exiting on an error.
        ** Note that a SB read error is not always going to result in an app quitting.
        */
        CFE_EVS_SendEvent(PM_PIPE_ERR_EID, CFE_EVS_ERROR,
			  "PM: SB pipe read error (0x%08X), app will exit", iStatus);
        g_PM_AppData.uiRunStatus= CFE_ES_APP_ERROR;
    }

    return (iStatus);
}
    
/*=====================================================================================
** Name: PM_ProcessNewData
**
** Purpose: To process incoming data subscribed by PM application
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    CFE_SB_RcvMsg
**    CFE_SB_GetMsgId
**    CFE_EVS_SendEvent
**
** Called By:
**    PM_RcvMsg
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    None
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void PM_ProcessNewData()
{
    int iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   TlmMsgPtr=NULL;
    CFE_SB_MsgId_t  TlmMsgId;

    /* Process telemetry messages till the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&TlmMsgPtr, g_PM_AppData.TlmPipeId, CFE_SB_POLL);
        if (iStatus == CFE_SUCCESS)
        {
            TlmMsgId = CFE_SB_GetMsgId(TlmMsgPtr);
            switch (TlmMsgId)
            {
                /* TODO:  Add code to process all subscribed data here 
                **
                ** Example:
                **     case NAV_OUT_DATA_MID:
                **         PM_ProcessNavData(TlmMsgPtr);
                **         break;
                */

                //

                case WISE_HK_TLM_MID:;
                        CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                                        "PM - Recvd WISE TLM (0x%08X)", TlmMsgId);
                        PM_ProcessWISEData(TlmMsgPtr);
                    break;
                default:
                    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                                      "PM - Recvd invalid TLM msgId (0x%08X)", TlmMsgId);
                    break;
            }
        }
        else if (iStatus == CFE_SB_NO_MESSAGE)
        {
            break;
        }
        else
        {
            CFE_EVS_SendEvent(PM_PIPE_ERR_EID, CFE_EVS_ERROR,
                  "PM: CMD pipe read error (0x%08X)", iStatus);
            g_PM_AppData.uiRunStatus = CFE_ES_APP_ERROR;
            break;
        }
    }
}
    
/*=====================================================================================
** Name: PM_ProcessNewCmds
**
** Purpose: To process incoming command messages for PM application
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    CFE_SB_RcvMsg
**    CFE_SB_GetMsgId
**    CFE_EVS_SendEvent
**    PM_ProcessNewAppCmds
**    PM_ReportHousekeeping
**
** Called By:
**    PM_RcvMsg
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    None
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void PM_ProcessNewCmds()
{
    int iStatus = CFE_SUCCESS;
    CFE_SB_Msg_t*   CmdMsgPtr=NULL;
    CFE_SB_MsgId_t  CmdMsgId;

    /* Process command messages till the pipe is empty */
    while (1)
    {
        iStatus = CFE_SB_RcvMsg(&CmdMsgPtr, g_PM_AppData.CmdPipeId, CFE_SB_POLL);
        if(iStatus == CFE_SUCCESS)
        {
            CmdMsgId = CFE_SB_GetMsgId(CmdMsgPtr);
            switch (CmdMsgId)
            {
                case PM_CMD_MID:
                    PM_ProcessNewAppCmds(CmdMsgPtr);
                    break;

                case PM_SEND_HK_MID:
                    PM_ReportHousekeeping();
                    break;

                /* TODO:  Add code to process other subscribed commands here
                **
                ** Example:
                **     case CFE_TIME_DATA_CMD_MID:
                **         PM_ProcessTimeDataCmd(CmdMsgPtr);
                **         break;
                */

                default:
                    CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                                      "PM - Recvd invalid CMD msgId (0x%08X)", CmdMsgId);
                    break;
            }
        }
        else if (iStatus == CFE_SB_NO_MESSAGE)
        {
            break;
        }
        else
        {
            CFE_EVS_SendEvent(PM_PIPE_ERR_EID, CFE_EVS_ERROR,
                  "PM: CMD pipe read error (0x%08X)", iStatus);
            g_PM_AppData.uiRunStatus = CFE_ES_APP_ERROR;
            break;
        }
    }
}
    
/*=====================================================================================
** Name: PM_ProcessNewAppCmds
**
** Purpose: To process command messages targeting PM application
**
** Arguments:
**    CFE_SB_Msg_t*  MsgPtr - new command message pointer
**
** Returns:
**    None
**
** Routines Called:
**    CFE_SB_GetCmdCode
**    CFE_EVS_SendEvent
**
** Called By:
**    PM_ProcessNewCmds
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    g_PM_AppData.HkTlm.usCmdCnt
**    g_PM_AppData.HkTlm.usCmdErrCnt
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void PM_ProcessNewAppCmds(CFE_SB_Msg_t* MsgPtr)
{
    uint32  uiCmdCode=0;

    if (MsgPtr != NULL)
    {
        uiCmdCode = CFE_SB_GetCmdCode(MsgPtr);
        switch (uiCmdCode)
        {
            case PM_NOOP_CC:
                g_PM_AppData.HkTlm.usCmdCnt++;
                CFE_EVS_SendEvent(PM_CMD_INF_EID, CFE_EVS_INFORMATION,
                                  "PM - Recvd NOOP cmd (%d)", uiCmdCode);
                break;

            case PM_RESET_CC:
                g_PM_AppData.HkTlm.usCmdCnt = 0;
                g_PM_AppData.HkTlm.usCmdErrCnt = 0;
                CFE_EVS_SendEvent(PM_CMD_INF_EID, CFE_EVS_INFORMATION,
                                  "PM - Recvd RESET cmd (%d)", uiCmdCode);
                break;

            case PM_TO_WISE_CAP_ACTIVE_CC:
		        g_PM_AppData.HkTlm.usCmdCnt++;		
               	PM_TO_WISE_CAP_ACTIVE_t *CmdPtr = (PM_TO_WISE_CAP_ACTIVE_t *) MsgPtr;
                send_whe_set_active_command(CmdPtr->setCap);
		        int value = CmdPtr->setCap; 
		        g_PM_AppData.HkTlm.actCap = value;
                CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "PM - Recvd ACTIVE CAP cmdId (%d) , Cap %d", uiCmdCode, value);
                break;;
            
            case PM_TO_WISE_CAP_DISCHARGE_CC:
		        g_PM_AppData.HkTlm.usCmdCnt++;
                CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "PM - Recvd DISCHARGE cmdId (%d)", uiCmdCode);
                send_discharge_command(CmdPtr->setCap);
                break;
            
           
            /* TODO:  Add code to process the rest of the PM commands here */

            default:
                g_PM_AppData.HkTlm.usCmdErrCnt++;
                CFE_EVS_SendEvent(PM_MSGID_ERR_EID, CFE_EVS_ERROR,
                                  "PM - Recvd invalid cmdId (%d)", uiCmdCode);
                break;
        }
    }
}
    
/*=====================================================================================
** Name: PM_ReportHousekeeping
**
** Purpose: To send housekeeping message
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    TBD
**
** Called By:
**    PM_ProcessNewCmds
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  GSFC, fsw
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void PM_ReportHousekeeping()
{
    /* TODO:  Add code to update housekeeping data, if needed, here.  */

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&g_PM_AppData.HkTlm);
    CFE_SB_SendMsg((CFE_SB_Msg_t*)&g_PM_AppData.HkTlm);
}
    
/*=====================================================================================
** Name: PM_SendOutData
**
** Purpose: To publish 1-Wakeup cycle output data
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    TBD
**
** Called By:
**    PM_RcvMsg
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void PM_SendOutData()
{
    /* TODO:  Add code to update output data, if needed, here.  */
    PM_ReportHousekeeping();
    // CFE_SB_TimeStampMsg((CFE_SB_Msg_t*)&g_PM_AppData.OutData);
    // CFE_SB_SendMsg((CFE_SB_Msg_t*)&g_PM_AppData.OutData);
}
    
/*=====================================================================================
** Name: PM_VerifyCmdLength
**
** Purpose: To verify command length for a particular command message
**
** Arguments:
**    CFE_SB_Msg_t*  MsgPtr      - command message pointer
**    uint16         usExpLength - expected command length
**
** Returns:
**    boolean bResult - result of verification
**
** Routines Called:
**    TBD
**
** Called By:
**    PM_ProcessNewCmds
**
** Global Inputs/Reads:
**    None
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
boolean PM_VerifyCmdLength(CFE_SB_Msg_t* MsgPtr,
                           uint16 usExpectedLen)
{
    boolean bResult=FALSE;
    uint16  usMsgLen=0;

    if (MsgPtr != NULL)
    {
        usMsgLen = CFE_SB_GetTotalMsgLength(MsgPtr);

        if (usExpectedLen != usMsgLen)
        {
            CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(MsgPtr);
            uint16 usCmdCode = CFE_SB_GetCmdCode(MsgPtr);

            CFE_EVS_SendEvent(PM_MSGLEN_ERR_EID, CFE_EVS_ERROR,
                              "PM - Rcvd invalid msgLen: msgId=0x%08X, cmdCode=%d, "
                              "msgLen=%d, expectedLen=%d",
                              MsgId, usCmdCode, usMsgLen, usExpectedLen);
            g_PM_AppData.HkTlm.usCmdErrCnt++;
        }
    }

    return (bResult);
}
    
/*=====================================================================================
** Name: PM_AppMain
**
** Purpose: To define PM application's entry point and main process loop
**
** Arguments:
**    None
**
** Returns:
**    None
**
** Routines Called:
**    CFE_ES_RegisterApp
**    CFE_ES_RunLoop
**    CFE_ES_PerfLogEntry
**    CFE_ES_PerfLogExit
**    CFE_ES_ExitApp
**    CFE_ES_WaitForStartupSync
**    PM_InitApp
**    PM_RcvMsg
**
** Called By:
**    TBD
**
** Global Inputs/Reads:
**    TBD
**
** Global Outputs/Writes:
**    TBD
**
** Limitations, Assumptions, External Events, and Notes:
**    1. List assumptions that are made that apply to this function.
**    2. List the external source(s) and event(s) that can cause this function to execute.
**    3. List known limitations that apply to this function.
**    4. If there are no assumptions, external events, or notes then enter NONE.
**       Do not omit the section.
**
** Algorithm:
**    Psuedo-code or description of basic algorithm
**
** Author(s):  fsw 
**
** History:  Date Written  2020-04-09
**           Unit Tested   yyyy-mm-dd
**=====================================================================================*/
void PM_AppMain()
{
    /* Register the application with Executive Services */
    CFE_ES_RegisterApp();

    /* Start Performance Log entry */
    CFE_ES_PerfLogEntry(PM_MAIN_TASK_PERF_ID);

    /* Perform application initializations */
    if (PM_InitApp() != CFE_SUCCESS)
    {
        g_PM_AppData.uiRunStatus = CFE_ES_APP_ERROR;
    } else {
        /* Do not perform performance monitoring on startup sync */
        CFE_ES_PerfLogExit(PM_MAIN_TASK_PERF_ID);
        CFE_ES_WaitForStartupSync(PM_TIMEOUT_MSEC);
        CFE_ES_PerfLogEntry(PM_MAIN_TASK_PERF_ID);
    }

    /* Application main loop */
    while (CFE_ES_RunLoop(&g_PM_AppData.uiRunStatus) == TRUE)
    {
        PM_RcvMsg(CFE_SB_PEND_FOREVER);
    }

    /* Stop Performance Log entry */
    CFE_ES_PerfLogExit(PM_MAIN_TASK_PERF_ID);

    /* Exit the application */
    CFE_ES_ExitApp(g_PM_AppData.uiRunStatus);
} 
    
/*=======================================================================================
** End of file pm_app.c
**=====================================================================================*/
    

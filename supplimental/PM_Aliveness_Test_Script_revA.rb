#########################################################
# Power Management (PM) Application Aliveness Test Script
# Date: 04/20/2020
#########################################################

# This script allows the user to command the PM application. It's intent is to provide the command interface and 
# report the command count and error count as the user commands the applicatino. This serves to proof the 
# application's health and intialization status. 

# Define command strings
cmd_noop = "CFS PM_NOOP_CC with CCSDS_STREAMID 6357, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 1, CCSDS_CHECKSUM 0, CCSDS_FC 0"
cmd_reset_pm = "CFS PM_NOOP_CC with CCSDS_STREAMID 6357, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 1, CCSDS_CHECKSUM 0, CCSDS_FC 1"
cmd_power_sbc = "CFS PM_TO_WISE_POWER_SBC_CC with CCSDS_STREAMID 6357, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 1, CCSDS_CHECKSUM 0, CCSDS_FC 4"
cmd_swap_active_cap = "CFS PM_TO_WISE_POWER_SBC_CC with CCSDS_STREAMID 6357, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 1, CCSDS_CHECKSUM 0, CCSDS_FC 2"
cmd_discharge_cap = "CFS PM_TO_WISE_POWER_SBC_CC with CCSDS_STREAMID 6357, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 1, CCSDS_CHECKSUM 0, CCSDS_FC 3"
pm_CmdCnt = tlm("CFS PM_HKMSG USCMDCNT")
pm_CmdErrCnt = tlm("CFS PM_HKMSG USCMDERRCNT")

#########
# START #
#########

# Ask to check current cmd count
value = message_box("Would you like to check the the current command count?", "No", "Yes", false)
case value
  when "Yes"
    puts ("Current Command Count: ")
    puts (pm_CmdCnt)
  when "No"
end

# Ask to reset the app
value = message_box("Would you like to reset the application ?", "No", "Yes", false)
case value 
  when "Yes"
    puts ("Resetting application")
    cmd(cmd_reset_pm)
    wait_check("CFS PM_HKMSG USCMDCNT == 0", 5)
    puts("Application has been reset")
  when "No"
end 

# Prompt to begin command sending verification loop
loop do
# print command and error count (updates the useras a response to the selection)
pm_CmdCnt = tlm("CFS PM_HKMSG USCMDCNT")
pm_CmdErrCnt = tlm("CFS PM_HKMSG USCMDERRCNT")
puts ("Current Command Count: ")
puts (pm_CmdCnt)
puts ("Current Command Error Count: ")
puts (pm_CmdErrCnt)
if pm_CmdErrCnt > 0
  message_box("WARNING: Application has reported a command error")
end
# Ask user what to do
value = vertical_message_box("What would you like to do?", "Send NOOP cmd", "Send Reset cmd", "Check Command Count", "Quit", false)
case value
  when "Send NOOP cmd"
    cmd(cmd_noop)
    wait(1)
  when "Send Reset cmd"
    cmd(cmd_reset_pm)
    wait(1)
  when "Check Command Count"
  when "Quit"
  break if value = "Quit"
end
end

# Quit, but ask to reset the app
value = message_box("Script exiting, would you like to reset the application ?", "No", "Yes", false)
case value 
  when "Yes"
    puts ("Resetting application")
    cmd(cmd_reset_pm)
    wait_check("CFS PM_HKMSG USCMDCNT == 0", 5)
    puts("Application has been reset")
  when "No"
end 

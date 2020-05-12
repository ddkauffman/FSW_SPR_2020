# Phase 4 Changes
* Updated install script to include WISE App and project configuration.
* Added: code to subscribe to the WISE application
* Created global variable to point to WISE HK TLM
* Added: Algorithms to
    * Select the active cap
    * Determine cap to discharge
    * Handle the SBC State
* Added: Helper functions to
    * find the cap with higest charge
    * find the cap with the min charge
    * send discharge command to wise
    * send active cap command to wise
    * get the rate of charge for a cap
    * get the number of failing caps
    * get the number of discharging caps
     
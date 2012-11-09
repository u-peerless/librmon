librmon
=======
Remote Monitoring is a very efficient way to know your network bandwidth. 
For details of RMON, see RFC 2819.
It is implemented in C. Four groups supported in this code repository.
They are as below:
  1. Statistics: real-time LAN statistics e.g. utilization, collisions, CRC errors
  2. History: history of selected statistics
  3. Alarm: definitions for RMON SNMP traps to be sent when statistics exceed defined thresholds
  4. Event: send alerts (SNMP traps) for the Alarm group or log the events.

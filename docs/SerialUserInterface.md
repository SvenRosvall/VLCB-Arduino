# Serial User Interface
This library provides a means of communicating with the module using its serial
interface.  The functions allow the VLCB switch and LEDs to be by-passed or not
included at all.  Additional functions that may be of use during development
are also available.

## Serial Monitor

By sending a single character to the Arduino using the serial send facility in the
serial monitor of the Arduino IDE (or similar),it is possible to initiate certain operations
or get information from the Arduino sketch.

| Command  | Description                                 |
|:--------:|---------------------------------------------|
|    n     | Show the node configuration.                |
|    e     | Show the learned event table in the EEPROM. |
|    v     | Show the node variables.                    |
|    h     | Show the event hash table.                  |
|    m     | Show the amount of free memory.             | 
|    *     | Reboot this node.                           |
|    s     | Enter setup mode.                           |

<!--- | r | (implementation pending) Let the node renegotiate its VLCB status by requesting a node number. The FCU will respond as it would for any other unrecognised module. | --->
<!--- | c | This character will return the CAN bus status.| -->

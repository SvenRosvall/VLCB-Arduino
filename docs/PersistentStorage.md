# Persistent Storage
This library stores module configurations, node variable and events in persistent storage.
There are a few different types of persistent storage that is supported by this
library such as EEPROM (internal and external via I2C) and Flash.

## Data Stored
Module configurations are stored in the persistent storage at particular addresses.
The address space is used are:

| Address | Description           |
|---------|-----------------------|
| 0       | Mode (SLiM=0, FLiM=1) |
| 1       | CAN ID                |
| 2       | NN high byte          |
| 3       | NN low byte           |
| 5       | Reset flag            |

Node Variables are stored as bytes from an address given by EE_NVS_START and 
uses EE_NUM_NVS bytes.
EE_NVS_START must be set to a value 10 or higher to make room for future additions.

Each event is stored as a structure with 4 bytes for Node Number and Event Number 
followed by one byte for each event variable. The number of event variables are defined
by EE_NUM_EVS. The bytes in the Event structure are:

| Offset | Description      |
|--------|------------------|
| 0      | NN high byte     |
| 1      | NN low byte      |
| 2      | EN high byte     |
| 3      | EN low byte      |
| 4      | Event Variable 1 |
| ...    | Any further EVs  |

The total number of bytes for each events are calculated and available in EE_BYTES_PER_EVENT.
There is storage available for EE_MAX_EVENTS events. 
Event data is starting at address EE_EVENTS_START and uses 
(EE_MAX_EVENTS * EE_BYTES_PER_EVENT) bytes.
EE_EVENTS_START must be set to a value larger than (EE_NVS_START + EE_NUM_NVS) to 
avoid data corruption.

## Storage Types
Different Arduino modules have different persistent storage.
Some have EEPROM on the processor, some have to use external EEPROM
or use flash memory. 
The library provides a choice of storage types.
See class diagram in the overall [design](Design.md) on how the different
storage classes fit into the general architecture.

The library also provides hooks for users to provide their own storage types such 
as an XML file stored on an SD card.
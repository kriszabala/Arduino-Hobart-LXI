# Arduino-Hobart-LXI
Uses Arduino as a piggyback controller to extend wash times on Hobart LXI commercial dishwashers.

I acquired a used Hobart LXI at auction and quickly realized commercial dishwashers are not like their household counterparts. Their quick cycle times (~2 mins) require all wares to be pre-scraped and soaked and act more like sanitizers rather than dishwashers. Since it was intended to be used for a catering business where the quick cycle time is not needed, I wanted to extend the wash cycle time to get more cleaning action. Unfortunately, Hobart did not make the cycle times a configurable parameter.

One option I considered was to reprogram the Motorola/NXP 68HC08 microcontroller on the control board but that would require a programmer that is not easy to come by these days. I then noticed that there were built in jumpers to activate the wash and rinse/fill relays and figured I could activate them on a timer to create a pre-wash cycle before activating the regular wash cycle.

In addition to the inconfigurable cycle times, I read that this series often had issues with fill levels, which my unit also exhibited. The machine would sometimes not re-fill enough water after every cycle. After troubleshooting and ruling out the water level sensors, I determined it was an issue with unreliable detection by the factory control logic. So I planned to address this issue as well with the piggyback controller.

The plan:
1. Intercept the wash button press signal.
2. Determine water fill level and add water if level is low.
3. Initiate a pre-wash cycle by activating the wash relay.
4. Trigger the factory wash cycle.

Conveniently the control board provides LED indicators for the current water level as well as the door sensor. I tapped into these to use as inputs for the piggyback controller to fill the water to the correct level and to shut down any functions if the door is opened for safety. 

Connections:

A -> NO4 - Relay 4 (Wash)
B -> COM4 - Relay 4 (Wash)
C -> NO3 - Relay 3 (Rinse)
D -> COM3 - Relay 3 (Rinse)
E -> Pin 5V - 5V
F -> Pin 9 - Water Level High
G -> Pin 10 - Water Level Med
H -> Pin 8 - Door Switch Input
I -> Ground - Ground
J -> Pin 12 - Rinse Sense Input
K -> Pin 11 - Wash Button Input 
L -> NO2 - Relay 2 (Wash Trigger)

Control Board Image

Working Features:
- Pre-fills to correct water level.
- Pre-wash cycle
- Rinse cycle detection to determine end of wash cycle.
- Wash button lockout mid cycle.
- Cycle shutdown when door opened.

Planned features:
- Detect wash button short or long press to be able to choose whether pre-wash is activated.
- Automatic wash start upon power up after detecting machine is warmed up fully.
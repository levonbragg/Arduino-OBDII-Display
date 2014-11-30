This is a simple project I created because I wanted an inexpensive 
OBD-II display for my Ford F250 Diesel. 

Yes, I know the code is not "Optimized" and is "Ugly". So what.
It works for me. Feel free to copy and clean-up for your own use. :)

This project uses an Arduino Mega, a SparkFun OBD-II adapter and a 4D Systems display.
You can change the code to add a software serial if you want to not use a mega.

I purchased a cheap ELM327 OBD-II to replace the SparkFun unit.
(I have not tested it yet with the ELM327)
I removed the BlueTooth module and tapped into the 
TTL Serial lines. This allows me to connect via TTL serial from the Arduino to the ELM327.

I further modified the ELM adapter to have a power switch, and a Ford HS / MS Can switch.
Here is a link to the MScan switch guide.
http://forscan.org/forum/viewtopic.php?f=4&t=4

The 4D Systems starter kit contains everything you need to start using the screen. (Cables, shield, SDcard, programmer...)

Included is both the Arduino INO and the 4D Systems project.

You can get the 4D Systems software here:
http://www.4dsystems.com.au/product/4D_Workshop_4_IDE/

I used this board kit:
http://www.4dsystems.com.au/product/uLCD_35DT_AR/
I purchased it from Mouser. It was about $10 cheaper.

You will also need the Arduino Libraryfrom 4D Systems:
https://github.com/4dsystems/ViSi-Genie-Arduino-Library

Future ToDo:
1. Drop the Arduino Mega and shield and use a Custom Teensey 3.1 or Program the Display directly...
 
 
hope you find it useful. If not, well at least it was free!

 

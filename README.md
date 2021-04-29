Here you'll find Arduino Sketches I used to make a small 2 wheel robot.  

Needed:  

M5Atom Motion  
2 5v DC motors  
A robot chassis (I used a Pololu Romi Chassis)  

Sketches:  

ATOM_Motion - this is the sketch provided by M5Stack to test the ATOM Motion  
ATOM_Motion_WebRemote - Setup an access point, connect to it, goto 192.168.4.1 and do some basic control of your robot. Forward, Back, Spin Left, Spin Right  
ATOM_Motion_WiFiRemote - Same as above, but connects to your router, you will need to get the IP address assigned to your ATOM and put that in the sketch for this to work.  It's a little buggie, but mostly works like this. (Unstable, random resets - This is low priority for me)  

JoyC Remote Robot:

Needed:
All of above.  
M5JoyC joystick  
M5StickC (should probably work with the M5StickC plus with some modifications)  

JoyC_Remote - this is the unmodified code provided by M5Stack for the M5Rover, use it on the M5StickC.  
Atom_Motion_Robot - This is a combination of the ATOM_Motion Sketch from above, and the "master" sketch for the M5Rover, this code is loaded to the Atom.  

The left stick will cause the robot to move forward back, make wide turns.  
While the right stick will cause the robot to spin  

Troubleshooting:  

If one or both of the motors are not going the correct direction (the ATOM_Motion_WebRemote sketch, is probably the easiest way to verify). Try swapping the wires on the motion connector for that motor.  

If the robot is turning left when you say turn right - try swapping the motor connections to the other side.  

Everything you need is included in the directories.  



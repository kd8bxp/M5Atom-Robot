The Wake up word is: "Hello Robot"

The recongizer needs to have a few phrases added.
Programmin using the phrase "Learn command word" or "Learning command word"

And should be in this order:

STOP command id 5
LEFT command id 6
RIGHT command id 7
BACKUP command id 8
SPEED UP command id 9
SLOW DOWN command id 10

When these have been successfully added, say "exit learning"

Documents:
https://wiki.dfrobot.com/SKU_SEN0539-EN_Gravity_Voice_Recognition_Module_I2C_UART#target_3

The commands for the robot will be:
Go Forward command id 22
stop command id 5
left command id 6
right command id 7
backup command id 8
speed up command id 9
slow down command id 10

because the recongizer maybe a little slow, the robot will be bound to a slower speed (50 to 80). 
Hopefully this will make the robot a little more controllable.

This is experimental, probably shouldn't be used in anytype of production.

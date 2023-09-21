# Iris SDK for Windows
## WARNING!! 
Be aware that the shaft or motor will move during operation. Position your hands and other sensitive assets accordingly. 
This firmware will cause the motor to create forces and motion. Be very careful at all times. Ensure the shaft and motor are mounted in a safe location and not in danger of hitting people or property.

## Description
This SDK abstracts the modbus communication protocol to allow for easy control and monitoring of Orca Series motors.
If high speed communications are desired, ensure you set the latency in the advanced settings of your RS422 comport to 1ms. 

Libraries are also available to make use of the IrisControls GUI interface
Iris Controls is a Windows application that connects over USB to Iris Dynamics devices such as the Orca series linear motors, and Raven kit.
It allows for monitoring and control of various parameters and features realtime plotting. 
Com0Com is a windows program which installs a virtual comport on your windows machine. This virtual comport is what allows the motor to communicate with Iris Controls. 

All user guides and reference manuals can be found at https://www.irisdynamics.com/downloads/ . 
The best place to get started is to read the SDK for Windows Quickstart Guide, which will give direction on device and developmnent environment setup. 
The Orca API User Manual describes how to command the Orca motor using the Actuator object that abstracts MODBUS communications.

Libraries: libraries which are used in projects. These include: 
* modbus_client - Actuator object/Orca API
* ic4_library - IrisControls4 API
* orca600_api
* irisSDK_libraries - useful objects and GUI panels which can be used in projects

Tutorials: a "Start_Here" projects that is the starting point of the tutorial guide as well as solution projects for each of the tutorials. 
Virtual Comport Setup: Setup program to set up com0com, the virtual comport utility which allows the windows virtual device to communicate with IrisControls4. 

## Good Luck!
We look forward to your feedback and hope this gets you on the path to success with our technology :)

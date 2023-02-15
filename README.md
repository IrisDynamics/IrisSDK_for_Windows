# Iris SDK for Windows
## WARNING!! 
Be aware that the shaft or motor will move during operation. Position your hands and other sensitive assets accordingly. 
This firmware will cause the motor to create forces and motion. Be very careful at all times. Ensure the shaft and motor are mounted in a safe location and not in danger of hitting people or property.

## Description
This SDK will demonstrate how to build a Windows Virtual Device, which can both communicate with an Iris Dynamics motor, and be monitored via Iris Controls.
Iris Controls is a Windows application that connects over USB to Iris Dynamics devices such as the Orca series linear motors, the Eagle, and the Super Eagle.
It allows for monitoring and control of various parameters and features realtime plotting. 
Com0Com is a windows program which installs a virtual comport on your windows machine. This virtual comport is what allows the motor to communicate with Iris Controls. 

All user guides and reference manuals can be found at https://www.irisdynamics.com/downloads/ . 
The best place to get started is to read the SDK for Windows Quickstart Guide, which will give direction on device and developmnent environment setup. 
The Orca API User Manual describes how to command the Orca motor using the Actuator object that abstracts MODBUS communications.

This repo contains the following:
Quickstart Guide: A guide on setting up the development environment and connecting a motor. 
Examples: pre written projects that demostrate features of IrisControls4 and this SDK.
IrisControls4: the Windows-based user-interface application used by Iris Dynamics Ltd. devices to present controls and data over USB. 
Libraries: libraries which are used in projects. These include: 
* modbus_client - Actuator object/Orca API
* ic4_library - IrisControls4 API
* orca600_api
* irisSDK_libraries - useful objects and GUI panels which can be used in projects
Tutorials: a "Start_Here" projects that is the starting point of the tutorial guide as well as solution projects for each of the tutorials. 
Virtual Comport Setup: Setup program to set up com0com, the virtual comport utility which allows the windows virtual device to communicate with IrisControls4. 

We recommend you begin by following instructions in the Quickstart Guide. 

##Cloning the Repo 
If you do not have GIT Bash on your machine, download and install it from https://git-scm.com/downloads 
Right click in the directory you would like to clone this repository into and select 'Git Bash Here'/ 
To clone this repo with all of the inlcuded submodules type the git bash command: 
<code>git clone https://github.com/IrisDynamics/IrisSDK_for_Windows</code>


## Good Luck!
We look forward to your feedback and hope this gets you on the path to success with our technology :)

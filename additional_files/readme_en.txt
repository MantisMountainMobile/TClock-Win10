***********************************************
***  Tclock-Win10 Ver 4.5.0.1 (2022/02/13)  ***
***       by Mantis Mountain Mobile         ***
***********************************************

Distribution via GitHub is started at Ver 4.5 on 2022/02/13
We are planning to keep releaseing on the Vector site (Japanese distributaion service).
Binalies after screening process by Vector is availabe on:
(As of February 13, 2022, Vector is experiencing service problems and updates will be delayed).
https://www.vector.co.jp/soft/winnt/personal/se518700.html

*******************
Updates in Ver. 4.5.0.1 (2022/02/13)

The main publishing method has been moved to GitHub.
Fixed the problem that the notification icon was not displayed correctly on Windows 11 depending on the color setting.

*******************

********
Introduction
********
This software is a TClock that runs on Windows 10 (Anniversary Update or later) and Windows 11.
Ver 4.0.1 supports Windows 11.

This software is based on the source code of TClock2ch final version (tclock-101021-analog) (by Kazubon => 2ch volunteer).
This software is based on the source code of
TClockLight-kt160911 (by Kazubon => K. Takata)
to support Windows 10 Anniversary Update, and now I have removed features that I think can be replaced by other software.
The graph display function of the base TClock2ch-analog can be used, and new functions such as network information acquisition have been added.

If you want to know the history of TClock software, which has been developed in a special form, please refer to the following sites.
TClock attached site
http://tclock2ch.no.land.to/

The existence of this program is due to Kazubon, K.Takata, and many other developers on the net.

This software is free software.

Since TClock and TClock Light are distributed under the GNU GPL, you may redistribute it and/or modify it under the terms of the GNU General Public License.

For inquiries about this software, please contact
mantis.mountain.mobile@gmail.com
Please note that we cannot answer questions about TClock software other than TClock-Win10.

************
Terms of Use
************
We will not bear any responsibility whatsoever for this software.

The base of this software is TClock2ch, which is a different system from TClockLight (currently TClockLight-kt). This software is based on TClock2ch, which is a different system from TClockLight series (currently TClockLight-kt). Please note that there are some differences between TClock2ch and TClockLight such as formatting.

This software is intended to be placed in a folder that can be operated by user privileges (e.g. in a user account).
Please use it in your personal folder, not in Program Files or any other Windows-controlled folder.
(It may also work in Program Files, but we have not checked it.)

This software may crash and loop involving Explorer due to configuration problems or compatibility with the environment.
In that case, you need to kill the process from the Task Manager, but "Safe Mode" is provided to facilitate the termination.
If the program was launched within 20 seconds of the last time it was launched, it may be able to be terminated by opening the right-click menu of TClock in order to stop and start the function that causes instability.
In this case, "[SafeMode]" will appear at the top of the task tray.

If you want to use Japanese in tooltips, please use Shift-JIS encoding.

[Known Issues]
It does not fully support dynamic changes in taskbar position and size. Please reboot TClock-Win10 if necessary.

If the number of connection profiles, including WiFi, exceeds a certain number (currently 128), the excess cannot be handled.
Please go to Windows Settings=>Network and Internet=>WiFi=>Manage Known Networks and delete the unused WiFi connection settings to reduce the number.

********************
How to upgrade
********************
Copy the following three files from the unzipped file to the existing TClock-Win10 folder. You can update the software while maintaining the previous settings.
01. tclock-win10.exe: Main program
02. tcdll-win10.dll: Taskbar modification module
03. tclang-win10.dll: Language module

****************
How to install
****************
Put the following files in the same folder and run TClock-Win10.exe.

[Required files (3)
01. TClock-Win10.exe:	Main program
02. tcdll-win10.dll:	Taskbar modification module
03. tclang-win10.dll:	Language module

[optional file]
04. tclock-win10.ini:	Configuration file. If not present, a new one will be created with default settings (recommended). Some configuration items require direct editing of this tclock-win10.ini file. 05. readme.txt: The readme.txt file.
05. readme_en.txt: 	This file.
06. tclock_tooltip.txt:	Tooltip content file. Default configuration file name. Sample files are included in the package. If you use Japanese, use Shift-JIS encoding. 07.
07. tclock.bmp: 	Analog clock background file. Default setting file name. A sample file is included. 08.
08. UnplugDrive.exe: executable file of the free software UnplugDrive Portable, not included in the distribution zip. put it directly under the TClock folder to use the removable drive removal function.

[Auto-generated file] 
09. TClock-Win10.log: 	This is a log of normal operation. If you don't need it, please open tclock-win10.ini and set NormalLog=0.
10. TCLOCK-WIN10-DEBUG.LOG: This is a log for debugging. However, it is not recommended to output the debug log on a daily basis because it increases disk access.

About formatting
The default setting is to use the standard display settings.
For extended formatting, we recommend that you first enter the following string in the custom format field of "Right-click Menu" => "Clock" => "Format" to see what information can be displayed, and then delete unnecessary displays to find a setting you like. (Please also enter the "" at the end.)
[ICP] WiFi LTE "U "NSSK___x "KB/s" Vol ADBCS "CPU "CU__x"%" WCSmm/dd ddd\nVPNS NMX1 "D "NRSK___x "KB/s" VL__xVMBL__x"%" CC___x "MHz" hh:nn:ss"

If the "Scaling and Layout" setting in the Windows display settings is set to anything other than 100%, the text in the right-click menu and settings dialog may be blurred.
In this case, open the properties of TClock-Win10.exe in Explorer and select "High" in the "Compatibility" tab.
Compatibility" tab and check "Disable screen scaling for high DPI settings".
Or
In the "Compatibility" tab, click "Change high DPI settings" and check "Override high DPI scaling behavior (scaling origin: application)
Check "Override high DPI scale (application)" in the "Compatibility" tab.

About VC++ Linetime Package
This software requires the VC++ runtime package.
If you get messages such as "The code execution cannot continue because VCRUNTIME140.dll is not found. ...","The code execution cannot continue because MSVCP140.dll is missing. ..." etc,please install Visual Studio 2015 Visual C++ Redistributable Package
https://www.microsoft.com/ja-jp/download/details.aspx?id=48145
(vc_redist.x64.exe for 64bit version).
(URL as of April 2019)

********************
How to Uninstall
********************
Erase the files from the folder.

The registry is not used.

The "Delete Registry" button in the right-click menu => "Tclock Properties" => "Other" tab is effective to delete all registry entries made by Tclock software in the past.

**********
Overview and Features
**********
The following is a list of features of TClock2ch that have been added after removing features that can be replaced by other software.

The graph display function, which was a feature of TClock2ch-analog, can be used.

Configuration management is now centralized in an ini file (tclock-win10.ini). The registry is no longer used.
The existing registry information deletion function is still available.

If the configuration file does not exist at the first startup, it will be created in the program folder with default settings.
If it fails to create the file, it will terminate the operation.

This application is built with Visual Studio 2015.

This application is designed to be used in a Japanese environment, but you can change the display language to English from the right-click menu.

Windows 10 is designed to connect to Ethernet, WiFi, and LTE (WAN) at the same time, and use Ethernet > WiFi > LTE for Internet connection in that order.

The SSID for WiFi connections and the access point name for WAN (LTE) connections can be displayed.
It also has a function to automatically switch the display according to the communication status (see NMX1 and NMX2 formats).

It can display the BL (battery level in %), VL&VM (volume and mute status), CU (CPU load), and communication speed bars in the background.

You can remove removable drives from the right-click menu. (UnplugDrive.exe required)

The indicator can be displayed for some VPN connections (VPN connection identification keyword setting is required in the configuration file).

**********************
How to use and change settings
**********************
[Functionality derived from TClock2ch].
For basic usage, please refer to the TClock attached site
http://tclock2ch.no.land.to/

For basic information on how to use TClock2ch, please refer to "Help for TClock2ch" at
http://tclock2ch.no.land.to/help2ch/

In particular, the old custom formatting is explained below. Some of them are no longer available.
http://tclock2ch.no.land.to/help2ch/template.html

At least the following points are different from the old version
TClock2ch was not able to detect the amount of WiFi network transmissions correctly, but it is now supported. TClock2ch was not able to detect the amount of WiFi network transmission and reception correctly.

The TTBase integration function and the desktop calendar integration function have been removed. It is no longer available.

In addition, the following functions have been removed	Window arrangement and operation functions
	Timer function
	Calendar function
	Web display function on tooltip
	Modification of the start button and start menu
	Multiple settings saving function
	Volume control function
	Time adjustment function
	Desktop icon modification function

Ini file
http://tclock2ch.no.land.to/help2ch/ini.html
TClock-Win10 is completely migrated to ini file (tclock-win10.ini), and ini file is created automatically.
TClock-Win10 will automatically create the ini file. If it fails to create the file, it will not start.

[The ini file will be created automatically.]
For the newly added functions that are unlikely to be changed, the configuration dialog is not implemented at this time.
Please edit tclock-win10.ini directly, and restart TClock.

If a VPN connection is not detected, please register a part of the network adapter name for VPN in "TClock Properties" => "Others" tab => "VPN Keywords" item.

You can switch the language of the right-click menu itself between Japanese and English by clicking "Language" at the top of the right-click menu.

If you can't identify the Ethernet connection, please register a part of the name of the Eithernet network adapter in "TClock Properties" => "Others" tab => Ethernet keyword.

**********************************
********** Safe Mode **********
**********************************
If the system starts up within 20 seconds of the last startup, it will assume that it is in a loop due to startup failure and enter safe mode.

This may avoid a situation where Explorer keeps crashing and TClock cannot be stopped except from Task Manager.
This may prevent Explorer from continuing to crash and TClock from being stopped except from Task Manager.

****************************************
TClock-Win10 modified and new features: formatting
****************************************

Some of the formats were added for debugging purposes, but I thought they could be useful.

[Version].
VerTC TClock-Win10 version string.

[Connection status/communication status indicator]
LTE When LTE connection is established, the string "LTE" is displayed*.
Communication status indicator with an asterisk (which becomes "LTE*") (default: 4 characters wide).
The string displayed when connecting can be changed in the LTEString in the ini file.

WiFi When the WiFi connection is established, the string "WiFi" will be displayed.
An asterisk ("WiFi*") will be displayed to indicate the communication status (5 characters wide).

EthS When Ethernet connection is established, the string "Eth" will be displayed.
Communication status will be displayed with an asterisk (becomes "Eth*").

EthL The string "Ethernet" will be displayed when the Ethernet connection is established.
Display the communication status with an asterisk (becomes "Ethernet*").

If you cannot identify the Ethernet connection, please register a part of the name of the Eithernet network adapter in "TClock Properties" => "Others" tab => Ethernet keyword item.

NMX1(NMX2) (* 10 characters wide) (The display is the same for both NMX1 and NMX2)
	Displays information about the currently active network communication path by switching automatically.
	When an Ethernet connection is established, the string "Ethernet" is displayed.
	Communication status is displayed with an asterisk ("Ethernet*").
	During WiFi Internet connection, SSID is displayed.
	During LTE(WAN) connection, the access point name is displayed.
The string width can be set in [ETC] NetMIX_Length in tclock-win10.ini.

SSID, APN (* 10 characters wide)
	Displays the SSID or access point name APN when a WiFi or LTE (WAN) connection has been established (InternetConnection).
	The SSID and access point name APN will be displayed when a connection is established (InternetConnection) not only on the currently active route but also in the background.
The string width can be set in [ETC] SSID_AP_Length in the tclock-win10.ini file.

When VPNS VPN is connected, "VPN" is displayed. When not connected, " " (3 characters wide)

EWLS Display Ethernet / WiFi / LTE connection status ("E*W*L*" with no space)(6 characters wide)

EWLL Display Ethernet / WiFi / LTE connection status (with space like "E* W* L*")(8 characters wide)

ICP Display the following information (1 character width)
Displayed characters
   - No Internet connection path
   E Internet connection path: Ethernet
   W Internet connection path: WiFi
   L Internet Connection Path: LTE(WAN)* M Internet Connection Path: Metered WiFi
   M Internet Connection Path: Metered WiFi

The display characters for LTE(WAN) in EWLS, EWLL and ICP format can be changed in LTEChar in the tclock-win10.ini file (only the first character is valid).

[Communication Profile Information]
WANP (WAN Profile) LTE (WAN) profile number in the connection profile (estimated if not connected).
DPRP (Data Plan Retrieved Profile) Profile number of the data communication usage to be retrieved in the connection profile.
AIPF (Active Internet ProFile) The profile number in the connection profile that is currently being used for Internet connection.

[IP Address (15 characters)
IPE Ethernet
IPW WiFi
IPL LTE (WAN)
IPV VPN
IPA Of the above, currently active

[Battery Charge Status]
"BCS" Battery Charge Status
Display "*" when battery is charging and " " when battery is not charging.

Mute Status Display Function] [Mute Status Display Function
VM In the mute state, the string (default: "*") set by MuteString in the ini file will be displayed.
When unmuted, the same number of spaces are displayed.

[FTA Flag Timer Adjustment Indicator]
FTA Flag Timer Adjustment Displays a "*" (usually " ") when the timer is reset (for one second only) to make the seconds display (somewhat) accurate.
Algorithmically, there is a limit to the accuracy of TClock's seconds display, so in many cases it is not worth worrying about.

[Accumulated traffic (extended)].
In addition to the conventional NSAK(KB), NSAM(MB), NRAK(KB), and NSRAM(MB) notations, the following formats have been added.
http://tclock2ch.no.land.to/help2ch/template.html
# NSAG = total amount sent (in GB)
# NRAG = total received (in GB)
For these two formats, the method of specifying digits is the same as before (minority display is possible).

In addition, the following automatic unit displays have been added
NSAA (NetSendAllAutodigit) Total amount sent from Tclock startup (in auto units)
NRAA (NetRecieveAllAutodigit) Total amount of data received since Tclock started (automatic unit)
The unit (KB=>MB=>GB) of four characters including the decimal point plus two half-width characters is attached.

[Memory Information (Extended)
For conventional displays such as MTPK, GB units can be displayed by setting the last character to "G".
In GB notation, the format is "#" type, which allows you to specify digits after the decimal point.
*In order to match the notation of Windows Task Manager, etc., these are calculated with 1GB=1024MB, regardless of the value of MegabytesInGigaByte in tclock.ini.
http://tclock2ch.no.land.to/help2ch/template.html

# MS = amount of physical memory (in GB, integer value, calculated as 1GB = 1000MB, available as a system installed memory notation)
# MG = amount of free physical memory (in GB) < same as MAPG >
# MTPG = amount of physical memory (in GB)
# MAPG = amount of free physical memory (in GB) < same as MG >
# MUPG = amount of physical memory used (in GB)
# MTFG = Amount of page file memory in GB
# MAFG = Amount of free pagefile memory (in GB)
# MUFG = Amount of memory used by page file (in GB)
# MTVG = virtual memory in GB
# MAVG = virtual free memory in GB
MAVG = virtual free memory (in GB) # MUVG = virtual used memory (in GB)

[Time difference setting]
Implemented a time difference format that mimics TClock Light.
It is written in the form of tdÅ}h:min. The "Å}" character must be a single-byte "+" or "-", and the hour and minute must be two single-byte digits, and time differences longer than 24 hours are invalid.
The time difference of more than 24 hours is invalid. The time difference is valid from one declaration to the next, and can be restored by writing "td+00:00" or "td-00:00".
The following is also implemented on a trial basis. Please note that this has not been fully tested.
tuÅ}h:min with North American Daylight Saving Time correction. The hour:minute should be the standard time difference.
teÅ}h:min with European Daylight Saving Time correction. The hour:minute should be the standard time difference.
StU, StE Daylight saving time indicator ("*" for daylight saving time, " " for normal). Reflects the time difference setting of the nearest tu or te before this format.

CUeXX, CCeXX: Load and clock for logical processor (thread) number XX (two-digit number). This function is the same as CCX and CUX, but now supports two-digit logical processor numbers.
It is not that accurate, so please consider it as a reference. The clock also does not match between two logical processors that share a physical core.
The description of the traditional format says core number, but it is actually a logical processor (thread). It is not possible to display each physical core.

PCORE: Number of physical cores
LPROC: Number of logical processors (threads)

***************
Version history
***************

Updates in Ver. 4.5.0.1 (2022/02/13)
The main publishing method has been moved to GitHub.
Fixed the problem that the notification icon was not displayed correctly on Windows 11 depending on the color setting.

Updates in Ver. 4.2.2.2 (2022/01/15)
Fixed a bug that December is not displayed correctly in "mmm" format.
Fixed a bug in which December was not displayed correctly in the "mmm" format. - The number of notifications can now be displayed on Windows 11, making it less likely that the display will shift.
Added support for sub-monitor display on Windows 11 (experimental).

Updates in Ver. 4.1.6.2 (2021/12/08)
Added the setting window for the text color of weekend/holiday and VPN connection ("Color Setting 2"). The "WCS" and "WCE" formats have been removed.
The keyword setting screen for various functions has been added ("Others" setting). For example, when wired LAN cannot be detected, please set "Ethernet Keyword".
It is now possible to start Windows standard "Alarm & Clock" by right-click menu or mouse click.
In case you want to match the time with another clock, you can shift the displayed time by a certain amount (up to 10 seconds).
Reviewed the tooltip display process and eliminated ineffective settings.
If you are concerned about delays in mouse clicking, please set the "Mouse Operation" setting to "None" for unneeded clicks.
Added support for the "Small Taskbar" display in Windows 11 by manipulating the registry. The problem of icons being shifted to the bottom in the small taskbar has also been improved (setting "Windows 11 support").
Added support for displaying the desktop from the right edge of the taskbar in Windows 11.

Updates in Ver. 4.0.3.5 (Nov. 24, 2021)
Provisional support for Windows 11. Manual adjustment of the values in the "Others & Win11" settings may be required for correct operation.
The taskbar clock of the sub-monitor can now be modified.
(The function of the sub-monitor clock is only available in the right-click menu. (The sub-monitor clock function is only available in the right-click menu, and is not yet implemented for Win11).
Added a setting to disable the tooltip display.
Added a setting to disable tooltip display.

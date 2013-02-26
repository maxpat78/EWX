EWX
===

This tiny C utility (less than 3K) drives the ExitWindows/ExitWindowsEx and SetSystemPowerState APIs and makes the User able to quit the current Windows session from command line.

It aims to be a replacement for the shell's standard point-and-click functions.

It is compatible with Windows 9x, NT, XP and Vista+.


Syntax
======

    EWX [-force] -logoff               ends the current's user session
    EWX [-force] -poweroff             exit Windows and turns off the power
    EWX [-force] -shutdown             exit Windows and turns off the power (Win 9x only)
    EWX [-force] -reboot               exit Windows and reboots the PC
    EWX -exit                          close the Desktop by ending the Explorer
    EWX -suspend                       turns the PC to low power consumption (retaining the session in memory)
    EWX -hibernate                     saves the session to disk and powers off
    
    
See inside BUILD.BAT about the different files to compile and link for the different target OSes.


The code is given to the Public Domain.

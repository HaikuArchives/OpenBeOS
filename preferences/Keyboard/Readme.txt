OpenBeOS Keyboard Preferences

23/02/2002 - Beta 3 Released

Re-written the program!
The code now fits OpenTracker guidelines.
Most of the work is done in Window and Application as it should be, and I have made much better use of BeOS's messaging features.
The KeyboardSettings class is tidier, with it only doing settings work, rather than window positioning too (Can you believe I put that code in there?? :) ) 
KeyboardView is derived from a BBox rather than a BView thanks to Philippe Houdoin for that one :)
Added a makefile for all you command line junkies out there.

15/02/2002 - Beta 2.0.1 Released.

Fixed a problem if the program is being run and there isn't a /boot/home/config/settings/Keyboard_Settings file already in place.

Bug #517797

02/02/2002 - Beta 2 Released.

This version:

Moved objects to private section of classes and created accessor methods.
Changed some messaging to read the data out of the message rather than going
back to the object for the information.
Added the icons.
Added an AboutRequested
If the initial co-ords are off screen, the application is now drawn in the middle of the screen.

To Do:

Font sensitivity (Delayed till R2)
...Probably loads of other stuff too!

This application is pretty much finished now, and really needs a good beta testing to try and spot bugs.

If anyone looks at my code and see's any major problems, please email me to tell me, I want to work hard on this app to make sure its as stable, and complete as it can get for the OpenBeOS project.

Andrew Edward McCall
mccall@digitalparadise.co.uk
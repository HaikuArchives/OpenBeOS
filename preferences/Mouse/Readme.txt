OpenBeOS Mouse Preferences

10/02/2002

Added the rest of the controls to the GUI.
Started implementing the Mouse Image.
- I may have to change the way the code is arranged. I probably don't need a MouseView class, and it will be easier if I build the BView inside the BWindow as it will mean I have easy access to information about the mouse without duplicating variables.
- I have decided to implement the Mouse image with bitmaps, yes I know this probably sucks, but it works.

09/02/2002

Changed the code to conform with the OpenTracker Style Guidelines
Changed the code to work with make

01/02/2002

All the file IO is in place
The messaging loops are there
Move over to make or Jam
Just need to create the GUI and hook it up to the backend
The "Mouse" image needs to be put in

If anyone looks at my code and see's any major problems, please email me to tell me, I want to work hard on this app to make sure its as stable, and complete as it can get for the OpenBeOS project.

Andrew Edward McCall
mccall@digitalparadise.co.uk
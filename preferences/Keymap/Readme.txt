OpenBeOS Keymap preferences

2002-02-15 - Added resource file and 'Use' button, made 'System' and 'User' lists mutually exclusive and implemented keymap file copying.

This version:
Actually copies keymap files!

To do (likely in this order):
- somehow inform Input Server to use new keymap
- fix map selection
- implement Revert button
- move message definitions to messages.h
- create member variable fApplication in KeymapWindow
- make the 'System' and 'User' labels show up (or: why are they invisible in the current version?)
- chop KeymapApplication::UseKeymap into smaller pieces
- layout the sources to conform to OBOS standards
- find Be-defined constants for paths and/or files
- check validity of keymap file before copying
- add all the fancy stuff that's in the original Keymap application
- switch to Jam
- get rid of all excess symbols (or: what linker switch makes the linker remove all external symbols?)
- hunt leaks and fix'em


If anyone looks at my code and sees any major problems, please email me to tell me, I want to work hard on this app to make sure it's as stable and complete as it can get for the OpenBeOS project.

Sandor Vroemisse
svroemisse@users.sf.net
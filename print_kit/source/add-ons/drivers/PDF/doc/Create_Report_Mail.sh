#!/bin/sh
/boot/apps/BeMail mailto:philippe.houdoin@free.fr -subject "pdf_writer v.0.33 report" -body "Thanks for taking the time to report how pdf_writer v0.3 worked for you!

Please edit the following text to reflect your experience when trying pdf_writer:

1 - How did pdf_writed worked?
It crashed/it produced grabage inside the window/it produced gargage all over the screen/it first worked, but stopped working when I did.../other...

2 - more information:
<put here some additionnal information/comments>


--------------------------------------------------------------------------
Please do not edit the information below, unless you don't want to disclose some information it contains:

-- Version app_server:
`/bin/version /system/servers/app_server`

-- Version print_server:
`/bin/version /system/servers/print_server`

-- Version libbe.so:	
`/bin/version /system/lib/libbe.so`

-- System print_server add-ons:
`/bin/ls -lt /boot/beos/system/add-ons/Print/*`

-- Custom print_server add-ons:
`/bin/ls -lt /boot/home/config/add-ons/Print/*`

-- System drivers:
`/bin/ls -lt /boot/beos/system/add-ons/kernel/drivers/bin/`

-- Custom drivers:
`/bin/ls -lt /boot/home/config/add-ons/kernel/drivers/bin/`
"


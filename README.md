dinputproxy
===========
dinputproxy is a proxy dll for DirectInput8

It allows blocking/blacklisting certain devices and device classes in DirectInput8.
Also if XONE controller enumerates as 1st person controller (I saw it at some point, buth then no longer; ¯\_(ツ)_/¯ ), it can be made to appear similar to a X360 controller.
See dinput8.ini for available options.

Personally I have used it in the game Bound by Flame, because the xbox controller was not recognized in-game. This was because the game accepted all HID devices in the system, and in my case some laptop temperature drivers had an HID extension.

By limiting the device querry to Mice, Keyboards and Gamecontroller IDs, problem was solved.

Also I remember Dark Sould 3 not picking-up the controller when my BT mouse was paired, and I couldn't be bothered to turn it off. The dll solved the issue.
In any case I have put in a blacklisting feature, where and unfortunate input device can also be hidden from the game (be it a mouse, a keyboard or any sort of device).

Other notes
-----------
N.A.

Disclamer of liability
---------
Some Anti-cheat solutions might flag this as cheating, so consider yourself warned. I personally try to avoid games with online component.

That being said, if you decide to use this tool, you are doing so at your own risk. I, and any persons involved in the development of this tool, cannot be held accountable for any damages caused by the use of the tool. My opinions, as expressed here, should not be taken as advice to how, YOU, or any other person, should or should not use this software.

Copyright
---------
dinputproxy is Copyright 2021 Mircea-Dacian Munteanu under the New BSD License,
see included LICENSE file.

dinputproxy contains the following third-party libraries:

- [inih](https://github.com/benhoyt/inih), which is copyright © 2009 – 2020 Ben Hoyt and used under a BSD 3-clause license.


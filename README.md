# DOS Defender

DOS Defender is an x86 real mode DOS game for [Ludum Dare #31][ld]
(December, 2014). Since few DOS machines still exist, the target
platform is actually [DOSBox][db], though it should work to some
extent on any DOS system. The game *can* be played with a mouse but
it's **intended to be played with a joystick/gamepad**.

![](http://i.imgur.com/B58ogsG.gif)

![](http://i.imgur.com/YQ82yLk.png)
![](http://i.imgur.com/IWJvurZ.png)

## Playing

Just point DOSBox at the COM binary.

    $ dosbox DOSDEF.COM

There are no external assets, so mounting isn't necessary.

## Building

All you need is a GCC compiler that can target i386. It doesn't matter
if it's for Linux, Windows, etc., nor is there a need for something
like DJGPP. The compiled code will be ripped out and re-packaged as a
COM file. The final game is a 10kB executable. To run, all it needs is
64kB of RAM and 64kB of video memory. With some tweaking, it could
even run without a need for any OS.

DOS Defender is written in GCC's dialect of C and probably can't be
built with any other compiler. The host platform is treated like an
embedded system, so there's no intention of actually building the game
from within DOS itself. Compilation is done as a single translation
unit to allow the optimizer full reign in minimizing the code size
(COM files are limited to 64kB).


[ld]: http://ludumdare.com/compo/2014/12/03/welcome-to-ludum-dare-31/
[db]: http://www.dosbox.com/

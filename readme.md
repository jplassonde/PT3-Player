# PT3-Player
Touchscreen music player on Cortex-M7 with FreeRTOS

[![PT3-Player](http://img.youtube.com/vi/uLp3UKVO3BU/0.jpg)](http://www.youtube.com/watch?v=uLp3UKVO3BU)

[Demo on YouTube](https://www.youtube.com/watch?v=uLp3UKVO3BU)

I didn't want to spend 150€ on ebay to get a Spectrum 128k and I thought it would be more fun to reverse engineer
the tracker file format and build my own touch/graphic engine from scratch. Right now it is still barely more than a 
proof-of-concept and there is a lot of things to rework and features to add.

-= Program Overview =-

It is built on STM32F769i-DISC0 in C++ with FreeRTOS and has 4 tasks. 
- One task get the data from the touchscreen controller on interrupt, process the data and send it to the main task 
through a queue.
- One update the display in two halfs to avoid diagonal tearing effect (the GRAM is not updated on the same axis as 
the screen refresh), and signal the availability of the backbuffer to the main task.
- One waits for commands through a queue and, (if playing), updates the content of the soundchip(s) on a 20ms timer.
- The main task treats touchscreen events, draws on the backbuffer and sends commands to the player task.


-= Custom Add-on Board =-

Two soundchips (AY8930 and AY-3-8913) are mounted and have their busses connected to the microcontroller through an 
I2C IO expander. A buffer is also present and acts as a level shifter for the clock and the reset lines. The ouput of
each chips 3 channels can be assigned to either the right, left or center channel through jump wires. A crude resistive
mixer is used to merge the channels together, since I'm an audiophile and I like my bleeps to be as noisy as they were originally 
unintended.

-= ProTracker 3.x parser =- 

The pt3 parser is still in an experimental stage. The tough part is that the file format if only briefly documented for an outdated
version (in Russian, which I don't speak <http://zxpress.ru/article.php?id=9313>), and there is quite a lot of edge cases and specific behaviors left out.

As of now all of the effects seem to be processing properly, without audio glitches, but additional testing of edge cases
is in order. There may be some mistakes remaining: off-by-one, wrong behaviors on overflow 8/16-bit, etc...
Now that most of it is figured out it has to be reworked in a more concise manner, since a few unexpected fixes forced me to bend
my original design.
Portamento effect works like the fixed version in PT3.6+, so files compiled with version <= 3.5 may sound different, if it was 
ever used in the few glitching cases.

------------------------------------------------------------------------------
-= Backlog =-

General: 
- Major refactoring and reworking of the core is essential before throwing
  anything else in that bucket of pasta. And dead/duplicate code removal. 
- Restrict scopes, Lots of them. Const decorating & fix some typecastings. 
- Implement proper DMA on I2C1 & SD card transfer.
- Software calibration of the touchscreen. Factory-mode "auto-calibration" is
  undocumented, and there is a significant drift at the end of the Y axis.
- Add track controls (pause play stop), display time, slider, etc...
- Create and add icons for files and folders in the browser.

PT3 parser:
- Refactoring
- Misc. fixes?
- Add Turbosound.

Others features to add (eventually, low priority):
- Add filebrowsing/playing via MQTT.
- Configure the board's audio codec and add a menu for volume & eq control.
- Add a mode for MIDI control, using pt3's samples and ornaments
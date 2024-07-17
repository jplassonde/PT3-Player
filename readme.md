# PT3-Player
Touchscreen music player on Cortex-M7 with FreeRTOS

[![PT3-Player](http://img.youtube.com/vi/uLp3UKVO3BU/0.jpg)](http://www.youtube.com/watch?v=uLp3UKVO3BU)

[Demo on YouTube](https://www.youtube.com/watch?v=uLp3UKVO3BU)

texxt
An attempt to build a touch-screen driven application without using a framework. 
Browse PT3 files (ZX Spectrum tracker format) on an SD card, parse & play them on 
AY-3-8910 soundchips on a custom add-on board.
It is currently a work in progress, the next step is to add file browsing through MQTT.


-= Program Overview =-

It is built on STM32F769i-DISC0 in C++ with FreeRTOS and currently has 4 tasks. 
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

-= Changelog =-

#### 30-04-2019
- Fixed the SD card DMA timeout for good, in the proper manner. The ST library ommitted to switch the SDMMC command path state machine back to idle state after operations. The timeout counter kept rolling and if no transaction was done in 3 minutes an interrupt was firing up during operation setup. A suggested workaround on ST community forum was to reinit and deinit the peripheral before and after each block(s) read, but it was a matter of finding the root of issue and the right bit to flip to come up with a much more efficient and elegant solution.
- Added Icons for files/folders
- More pointers! Smart pointers everywhere! The player now receive a directory object which stay (mostly) shared with the browser until either of them switch to a new one. This way the player is more independant, can receive from multiple source (MQTT ready) and can switch to next track by itself.
- Total time & elapsed time now show up on the player-display, along with a slider which can be used to seek to a certain position in the track. An intermediate object between the player and the player-display is used for communication.
- Tabs to switch between the player-display and and the browser.
- Improved the sine-color formula for the player-display.
- Fixed the last remaining memory leak! (more to come... :P)

------------------------------------------------------------------------------
-= Backlog =-

General: 
- Software calibration of the touchscreen. Factory-mode "auto-calibration" is
  undocumented, and there is a significant drift at the end of the Y axis.
- Add track controls (pause play stop).

PT3 parser:
- Hunt down bug, and rewrite some parts...

Others features to add (eventually, low priority):
- Add filebrowsing/control via MQTT.
- Add a mode for MIDI control, using pt3's samples and ornaments
- Configure the board's audio codec and add a menu for volume & eq control.
# PT3-Player
Touchscreen music player on Cortex-M7 with FreeRTOS

[![PT3-Player](http://img.youtube.com/vi/azNRBfbVEs0/0.jpg)](https://www.youtube.com/watch?v=azNRBfbVEs0)

[Quick demo on YouTube](https://www.youtube.com/watch?v=azNRBfbVEs0)

A touch-screen driven application with a homebrewed UI, heavy dynamic allocation and smart pointers usage in C++. 
Browsing PT3 files (ZX Spectrum tracker format) on an SD card, parse & play them on AY-3-8910 soundchips on a custom add-on board.
The outputs of the sound generators is mixed passively and sent to the audio codec, where gating and compression are applied to filter unwanted noise and make it sound more crunchy. From there it is sent to the headphone jack and to the MCU via i2s, which then send the audio via USB if headset is connected.

### Program Overview

It is built on STM32F769i-DISC0 in C++ with FreeRTOS and has 4 tasks. 
- One task get the data from the touchscreen controller on interrupt, process the data and send it to the main task 
through a queue.
- One update the display in two halfs to avoid diagonal tearing effect (the GRAM is not updated on the same axis as 
the screen refresh), and signal the availability of the backbuffer to the main task.
- One waits for commands through a queue and if playing, updates the content of the soundchip(s) on a 20ms timer.
- The main task treats touchscreen events, draws on the backbuffer and sends commands to the player task.


### Custom Add-on Board

Two soundchips (AY8930 and AY-3-8913) are mounted and have their busses connected to the microcontroller through an 
I2C IO expander. A buffer is also present and acts as a level shifter for the clock and the reset lines. The ouput of
each chips 3 channels can be assigned to either the right, left or center channel through jump wires. A crude resistive
mixer is used to merge the channels together, then an audio jack is connecting to the STM32 board's audio codec.

### Key challenges I overcame during this project include:
* Fixes were needed in the STM32 USB Host library. Issues in the state machines broke the freertos compatibility, and other modifications were required to allow composite devices and send both audio stream and led update to the headset.
* Undocumented hardware bug in one of the sound generator hardware (AY-3-8913). Seemingly random ticking. After investigation  the frequency counter was not behaving as mentioned in datasheet. Up counter instead of down counter, == comparison instead of >=. Changing to a lower value than the current count was making it overflow and a long pause was therefore messing up the cycle.
* Fixing a HAL library issue with SD card DMA timeout, the right way. The ST library ommitted to switch the SDMMC command path state machine back to idle state after operations. The timeout counter kept rolling and if no transaction was done in 3 minutes an interrupt was firing up during operation setup. A suggested workaround on ST community forum was to reinit and deinit the peripheral before and after each block(s) read, but it was a matter of finding the root of issue and the right bit to flip to come up with a much more efficient and elegant solution.
* Parsing files from a very brief file format specification written in russian and google translate. Most edge cases, behavior on overflow for 8 and 16 bit values had to be deducted looking at a hex editor, debugger output and trials and errors.
* Dealing with the feature creep. The project started as an experimentation with a display and progressively turned into something bigger. Bad early design choices forced me to think of ways to find a middle ground between refactoring all of it or ending with too tight coupling in many places.



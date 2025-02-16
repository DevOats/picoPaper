# PicoPaper
A framework to display images on a WaveShare e-paper display with a PC, using a Raspberry Pi Pico 2.  
This framework was written for my own need and use case. I published it hoping others will benefit from this as well. Please have a look at the MIT license below and know that there's no warranty of any kind.  
  
Happy coding:)   
Joost

## Requirements and support
- Raspberry Pi Pico 2 W
- Waveshare 7.5" Black&White ePaper display
- VsCode with the official Raspberry Pi Pico C SDK extension 
- .NET 8
- Display image:
    - Bitmap (*.bmp)
    - Should always be 800 x 480 pixels
    - Only pure white pixels in the bitmap (RGB: 0xFFFFFF) are converted to white pixels on the display. Everything else will become black.

## How to make this work:
- Connect a Raspberry Pi Pico 2W to a Waveshare 7.5" Black&White e-Paper display using their driver board.
- Program the Raspberry Pi Pico with the source code from the pipico folder
    - Use the official Raspberry Pi Pico C SDK vscode extension
- Figure out which virtual serial port is assigned to the Pi Pico (e.g. COM4)
- Use one of two ways to upload an image to the Pi Pico:
    - Use the command line application (picoPaper.exe) to upload a bitmap file
    - Use the .NET library assembly API (picoPaperLib.dll) to send a bitmap from your own .NET application.


## Notes
- This repository contains reference code from WaveShare. Many thanks to this company for their support.
- When using ePaper displays outside, to prevent permanent damage:
    - Keep them away from direct sunlight
    - Use a good UV filter
- For display longevity: When turning off your application, it's best to first clear the display.

## MIT License
```
MIT License

Copyright (c) 2025 Joost Haverkort/DevOats

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

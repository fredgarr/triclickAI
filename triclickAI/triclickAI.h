#ifndef TRICLICK_H
#define TRICLICK_H
/*
MIT License
Copyright (c) 2019 fredgarr
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
Original code: Debounce.ino
modified 8 May 2014  by Scott Fitzgerald   modified 2 Sep 2016
by Arturo Guadalupi   modified 8 Sep 2016   by Colby Newman
This code is in the public domain.
*/

// #define DEBUG

// Functional constants:
//--------------------------------------------

// Maximum time window duration authorized for the N clicks (in ms)
// Time margin 50ms
// FIXME: Some *2 clock error requiering to divide timer by 2...
//        should be 3000 to get 3 seconds
#define CLICKS_DURATION 3000
#define TIME_MARGIN  150

// Max number of cliks authorised per time window
#define NUMBER_OF_CLICKS 3
#define NUMBER_OF_TRANSITIONS     (NUMBER_OF_CLICKS*2)

// Debounce timer expire return
#define EXPIRED_TIMER 0xFFFFFFFF
#define DISABLE_TIMER 0xFFFF


// Misc:
//--------------------------------------------
#define True 1
#define False 0

typedef unsigned long uint32_t;
typedef long int32_t;
typedef unsigned int uint16_t;
typedef int int16_t;
typedef unsigned char uint8_t;

#endif //TRICLICK_H
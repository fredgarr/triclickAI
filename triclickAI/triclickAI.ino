/*
  TriclickAI

  Will record a pressed button transitions and send it to the PC 
  through serial interface when 3 CLicks have been detected in less 
  than 3seconds

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
#include "triclickAI.h"

// constants won't change. They're used here to set pin numbers:
const uint16_t buttonPin = 2;    // the number of the pushbutton pin
const uint16_t ledPin = PIN_A3;    //13  // the number of the LED pin

// Variables will change:
uint16_t buttonState = LOW;       // the current reading from the input pin
uint16_t lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
// Note: milliseconds counter overflows every ~50 days.
uint32_t lastDebounceTime = 0;  // the last time the output pin was toggled
uint32_t debounceDelay = 5;    // the debounce time; 
                                    // EndOfCourse switch bounce time measured to 1ms
                                    // Set to 3 ms for security


// blinkLed: 
// BLinks the led for a given number of time.
// @param[in]  times : Number of blink 
// @param[out] None
void blinkLed(int times) {

    uint16_t k;
    for(k=0; k<times; k++)
    {
        analogWrite(ledPin, 255);
        delay(50);
        analogWrite(ledPin, 0);
        delay(50);
    }
    // Power off the LED at the end:
    analogWrite(ledPin, 0);
}


// debounce: 
// Returns a time value when a stabilized contactor
// modification has been detected. Either 0->1 or 1->0
// @param[in]  timeout : in milliseconds, will return even if no action after maxtime with EXPIRED_TIMER value.
//                       mainly used for calibration part.
// @param[out] Time value
uint32_t debounce(uint16_t timeout)
{
    uint16_t reading = 0;
    uint16_t switchState = False;
    uint32_t endTime = millis() + (uint32_t)timeout;
    uint32_t returnTimer=  0;

    #ifdef DEBUG
    char  message[128];               // for print formating
    sprintf(message, "debounce %d", timeout);
    Serial.println(message);
    #endif

    while(switchState == False)
    {  
        reading = digitalRead(buttonPin);
        // If the switch changed, due to noise or pressing:
        if (reading != lastButtonState) 
        {
            lastDebounceTime = millis();
        }
            
        // FIXME : Manage 50days overflow
        if ((millis() - lastDebounceTime) > debounceDelay) 
        {
            // whatever the reading is at, it's been there for longer than the debounce
            // delay, so take it as the actual current state:

            // if the button state has changed:
            if (reading != buttonState) 
            {
                buttonState = reading;

                // Steady state reached, exit loop
                switchState = True;
                returnTimer = millis();
            }
        }
        // save the reading. Next time through the loop, it'll be the lastButtonState:
        lastButtonState = reading;

        // Check if timout is required
        // if yes and expired, return, FFFFFFFF
        if(timeout != DISABLE_TIMER)
        {
            if(endTime < millis())
            {
                switchState = True;
                returnTimer = EXPIRED_TIMER;
            }
        }
    }
    return returnTimer;
}


void waitButtonReleased(const char* pMessage)
{
    while(digitalRead(buttonPin)!=LOW)
    {
        debounce(500);
        if(pMessage != NULL)
        {
            Serial.println(pMessage);
        }
    }
    // Reset Global value:
    // FIXME: Use calss instead of global...
    lastButtonState = LOW;
    delay(50);
}


void doCheckCommand()
{
    uint16_t k;
    uint32_t newTime;
    uint32_t referenceTime;
    uint16_t timeOut;
    uint16_t cmdDetected = 1;
    uint8_t  message[128];               // for print formating
    
    uint32_t edgeTable[NUMBER_OF_TRANSITIONS];

    memset(edgeTable, 0, NUMBER_OF_TRANSITIONS);

    waitButtonReleased(NULL);

    // Wait first edge, unlimited time:
    referenceTime = debounce(DISABLE_TIMER);
    newTime = referenceTime;

    // Wait for next edge with timeout management
    timeOut = CLICKS_DURATION;

    // Then record next transitions
    for(k=1; k<NUMBER_OF_TRANSITIONS; k++)
    {

        newTime = debounce((timeOut));

        if(EXPIRED_TIMER == newTime)        
        {
            // sprintf((char*)message, "TimeOut, reset sequence (%d, %d)", (uint16_t)newTime,timeOut);
            // Serial.println((char*)message);
            blinkLed(5);
            return;
        }

        timeOut = (uint16_t)(CLICKS_DURATION - newTime + referenceTime);
        newTime -= referenceTime;

        edgeTable[k] = newTime;
        cmdDetected++;
    }

    if(cmdDetected == NUMBER_OF_TRANSITIONS)
    {
        sprintf((char*)message, "%d, %d, %d, %d, %d",
            (uint16_t)edgeTable[1],
            (uint16_t)edgeTable[2],
            (uint16_t)edgeTable[3],
            (uint16_t)edgeTable[4],
            (uint16_t)edgeTable[5]);
        Serial.println((char*)message);
    }

}

void setup() {
    // init serial Port for debugging
    Serial.begin(115200);

    // Init pin in/out
    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);

    // set initial LED state
    digitalWrite(ledPin, LOW);

}

void loop() {
    
    // Check commands
    doCheckCommand();

}
#include <CapacitiveSensor.h>

// This suppress the pragma warning of FastLED (see https://github.com/FastLED/FastLED/issues/797)
#define FASTLED_INTERNAL
#include "FastLED.h"

//#define DEBUG 1

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
#endif

// TBD What is the right type for me
#define LED_TYPE WS2812B
// TBD what does this mean??
#define COLOR_ORDER GRB
// TBD what is the range is 64 max?
#define DEFAULT_BRIGHTNESS 64

// TBD this should be nicer
#define MAX_NUM_LEDS    60
#define SENSE1_NUM_LEDS 60
#define SENSE2_NUM_LEDS 60
#define SENSE3_NUM_LEDS 60
#define SENSE4_NUM_LEDS 60

#define PIN_TOUCH_SEND     4
#define SENSE1_PIN_RECEIVE 5
#define SENSE1_PIN_LED     6
//#define SENSE2_PIN_RECEIVE 7
//#define SENSE2_PIN_LED     8
//#define SENSE3_PIN_RECEIVE 9
//#define SENSE3_PIN_LED     10
//#define SENSE4_PIN_RECEIVE 10
//#define SENSE4_PIN_LED     11

#define SENSE_MEASUREMENT_CYCLES 10

#define SENSE_STORED_PER_LED 10000
#define SENSE_STORED_PER_LED_100 100
#define DECREASE_SENSE_SPEED 400
#define INCREASE_SENSE_SPEED 200
#define CYCLE_INTERVAL 40 // 41.666 == 24 update / sec

long lastCycle = millis();

CRGB fade(CRGB from, CRGB to, int percent) {
    CRGB result;
    result.r = from.r + ((to.r - from.r) / 100) * percent;
    result.g = from.g + ((to.g - from.g) / 100) * percent;
    result.b = from.b + ((to.b - from.b) / 100) * percent;
    return result;
}

struct TouchLedStrip {
    const uint8_t sendPin;
    const uint8_t sensePin;
    const uint8_t ledPin;
    const uint8_t numLeds;
    CRGB activeBackColor = CRGB::White;
    CRGB inactiveBackColor = CRGB::Black;
    CRGB runnerColor = CRGB::Blue;

    CapacitiveSensor sensor;
    CRGB             leds[MAX_NUM_LEDS];

    long startTime;
    long accSense;
    int curRunner;

    unsigned long binaryUpdate;
    unsigned long ledUpdate;

    TouchLedStrip(int sendPin, int sensePin, int ledPin, int numLeds) : sendPin(sendPin), sensePin(sensePin), ledPin(ledPin),numLeds(numLeds), sensor(sendPin,sensePin) { }
    void doSetup() { sensor.set_CS_AutocaL_Millis(0xFFFFFFFF); }
    long sense() { return sensor.capacitiveSensor(SENSE_MEASUREMENT_CYCLES); }
    CRGB backColor(int ledIndex) {
        int toggleLed = accSense / SENSE_STORED_PER_LED;
            DEBUG_PRINT("toggleLed: ");
            DEBUG_PRINTDEC(toggleLed);
            DEBUG_PRINTLN("");
        if (toggleLed > ledIndex) {
            return activeBackColor;
        } else if (toggleLed < ledIndex ){
            return fade(inactiveBackColor, activeBackColor, 2);
        } else {
            int senseRest = accSense % SENSE_STORED_PER_LED;
            if ( senseRest == 0 ) {
            return fade(inactiveBackColor, activeBackColor, 2);
            }
            int colorPercent = senseRest / SENSE_STORED_PER_LED_100;
            DEBUG_PRINT("accSense: ");
            DEBUG_PRINTDEC(accSense);
            DEBUG_PRINT(" senseRest: ");
            DEBUG_PRINTDEC(senseRest);
            DEBUG_PRINT(" colorPercent: ");
            DEBUG_PRINTDEC(colorPercent);
            DEBUG_PRINT(" SENSE_STORED_PER_LED_100: ");
            DEBUG_PRINTDEC(SENSE_STORED_PER_LED_100);
            DEBUG_PRINTLN("");
            return fade(inactiveBackColor, activeBackColor, colorPercent);
        }
    }
    void runCycle() {
        // Process the sense event
        int senseVal = sense();
        if ( senseVal < 1000 && senseVal > -1000) {
            curRunner = 0;
            // TBD let is slowly the accSense decrease
            if ( accSense < DECREASE_SENSE_SPEED ) {
                accSense = 0;
            } else {
                accSense -= DECREASE_SENSE_SPEED;
            }
            startTime = 0;
        } else {
            if (curRunner < numLeds * 2) {
              curRunner++;
            } else {
              curRunner = 0;
            }
            curRunner++;
            accSense += INCREASE_SENSE_SPEED;
            if ( startTime == 0 ) {
                startTime = millis();
            } 
        }
        DEBUG_PRINT("startTime: ");
        DEBUG_PRINTDEC(startTime);
        DEBUG_PRINT("; senseVal: ");
        DEBUG_PRINTDEC(senseVal);
        DEBUG_PRINT("; accSense: ");
        DEBUG_PRINTDEC(accSense);
        DEBUG_PRINT("; ledPin: ");
        DEBUG_PRINTDEC(ledPin);
        DEBUG_PRINTLN("");
        for ( int i = 0; i < numLeds; i++ ) {
            CRGB bg = backColor(i);
            if ( curRunner / 2 == i  || (curRunner / 2) + 1 == i) {
              leds[i] = runnerColor;
            } else {
              leds[i] = bg;              
            }
        }
    }
};


TouchLedStrip touchLedStrips[]{
    TouchLedStrip(PIN_TOUCH_SEND, SENSE1_PIN_RECEIVE, SENSE1_PIN_LED, SENSE1_NUM_LEDS) };
//    TouchLedStrip(PIN_TOUCH_SEND, SENSE2_PIN_RECEIVE, SENSE2_PIN_LED, SENSE2_NUM_LEDS), 
//    TouchLedStrip(PIN_TOUCH_SEND, SENSE3_PIN_RECEIVE, SENSE3_PIN_LED, SENSE3_NUM_LEDS), 
//    TouchLedStrip(PIN_TOUCH_SEND, SENSE4_PIN_RECEIVE, SENSE4_PIN_LED, SENSE4_NUM_LEDS) };


void setup()
{
#if DEBUG
    Serial.begin(9600);
#endif

    for (int index = 0; index < sizeof(touchLedStrips)/sizeof(TouchLedStrip); index++) {
        touchLedStrips[index].doSetup();
        // Workaround for the template
        switch(touchLedStrips[index].ledPin) {
            case SENSE1_PIN_LED:
                FastLED.addLeds<LED_TYPE, SENSE1_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                break;
//            case SENSE2_PIN_LED:
//                FastLED.addLeds<LED_TYPE, SENSE2_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
//                break;
//            case SENSE3_PIN_LED:
//                FastLED.addLeds<LED_TYPE, SENSE3_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
//                break;
//            case SENSE4_PIN_LED:
//                FastLED.addLeds<LED_TYPE, SENSE4_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
//                break;
        }
    }
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

void loop()
{
    for (int index = 0; index < sizeof(touchLedStrips)/sizeof(TouchLedStrip); index++) {
        touchLedStrips[index].runCycle();
    }
   
    FastLED.show();
    
    long timeRemain = (lastCycle + CYCLE_INTERVAL) - millis();
    if (timeRemain > 0) {
        delay(timeRemain);
    }
    DEBUG_PRINT("timeRemain: ");
    DEBUG_PRINTDEC(timeRemain);
    DEBUG_PRINTLN("");
    lastCycle = millis();
}

////        if (millis() - binaryUpdate > PULSE_SPEED) {
////            binaryUpdate += PULSE_SPEED;
////            for (int i = numLeds - 1; i > 0; i--) {
////                binaryLedState[i] = binaryLedState[i - 1];
////                FastLED.show();
////            }
////        }
////        if(millis()-ledUpdate > FADE_SPEED) {
////            ledUpdate += FADE_SPEED;
////            fade()
////        }
////    }
////    void fade() {
////        for(int b=0; b < NUM_LEDS; b++){ // over the whole lenght
////            leds[b].r = (int)old_frame[b][0] + ((int)new_frame[b][0] - (int)old_frame[b][0]) * a / steps; //over the red
////            leds[b].g = (int)old_frame[b][1] + ((int)new_frame[b][1] - (int)old_frame[b][1]) * a / steps; //the green
////            leds[b].b = (int)old_frame[b][2] + ((int)new_frame[b][2] - (int)old_frame[b][2]) * a / steps; //and the blue LEDs
////        }
////    }
//// Helper function that blends one uint8_t toward another by a given amount
//void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount)
//{
//    if( cur == target) return;
//
//    if( cur < target ) {
//        uint8_t delta = target - cur;
//        delta = scale8_video( delta, amount);
//        cur += delta;
//    } else {
//        uint8_t delta = cur - target;
//        delta = scale8_video( delta, amount);
//        cur -= delta;
//    }
//}
//
//// Blend one CRGB color toward another CRGB color by a given amount.
//// Blending is linear, and done in the RGB color space.
//// This function modifies 'cur' in place.
//CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount)
//{
//    nblendU8TowardU8( cur.red,   target.red,   amount);
//    nblendU8TowardU8( cur.green, target.green, amount);
//    nblendU8TowardU8( cur.blue,  target.blue,  amount);
//    return cur;
//}
////fade to new content over a numer of steps with wait_ms delay between the steps
//void fade_from_old_to_new_frame(int steps, int wait_ms){
//    for(int a=0; a < steps; a++){ // over the steps
//        for(int b=0; b < NUM_LEDS; b++){ // over the whole lenght
//            leds[b].r = (int)old_frame[b][0] + ((int)new_frame[b][0] - (int)old_frame[b][0]) * a / steps; //over the red
//            leds[b].g = (int)old_frame[b][1] + ((int)new_frame[b][1] - (int)old_frame[b][1]) * a / steps; //the green
//            leds[b].b = (int)old_frame[b][2] + ((int)new_frame[b][2] - (int)old_frame[b][2]) * a / steps; //and the blue LEDs
//        }
//        FastSPI_LED.show();
//        delay(wait_ms);
//    }
//}

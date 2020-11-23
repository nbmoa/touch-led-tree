#include <CapacitiveSensor.h>

// This suppress the pragma warning of FastLED (see https://github.com/FastLED/FastLED/issues/797)
#define FASTLED_INTERNAL
#include "FastLED.h"

// TBD What is the right type for me
#define LED_TYPE WS2811
// TBD what does this mean??
#define COLOR_ORDER GRB
// TBD what is the range is 64 max?
#define DEFAULT_BRIGHTNESS 64

// TBD this should be moved to a struct
#define SENSE1_NUM_LEDS 60
#define SENSE2_NUM_LEDS 60
#define SENSE3_NUM_LEDS 60
#define SENSE4_NUM_LEDS 60

#define PIN_TOUCH_SEND     3
#define SENSE1_PIN_RECEIVE 4
#define SENSE1_PIN_LED     5
#define SENSE2_PIN_RECEIVE 6
#define SENSE2_PIN_LED     7
#define SENSE3_PIN_RECEIVE 8
#define SENSE3_PIN_LED     9
#define SENSE4_PIN_RECEIVE 10
#define SENSE4_PIN_LED     11

CapacitiveSensor sense1Sensor                = CapacitiveSensor(PIN_TOUCH_SEND, SENSE1_PIN_RECEIVE);
CRGB             sense1Leds[SENSE1_NUM_LEDS];
CapacitiveSensor sense2Sensor                = CapacitiveSensor(PIN_TOUCH_SEND, SENSE2_PIN_RECEIVE);
CRGB             sense2Leds[SENSE2_NUM_LEDS];
CapacitiveSensor sense3Sensor                = CapacitiveSensor(PIN_TOUCH_SEND, SENSE3_PIN_RECEIVE);
CRGB             sense3Leds[SENSE3_NUM_LEDS];
CapacitiveSensor sense4Sensor                = CapacitiveSensor(PIN_TOUCH_SEND, SENSE4_PIN_RECEIVE);
CRGB             sense4Leds[SENSE4_NUM_LEDS];

void setup()
{
#if DEBUG_SERIAL
    Serial.begin(9600);
#endif
    sense1Sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    sense2Sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    sense3Sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    sense4Sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    
    FastLED.addLeds<LED_TYPE, SENSE1_PIN_LED, COLOR_ORDER>(sense1Leds, SENSE1_NUM_LEDS); 
    FastLED.addLeds<LED_TYPE, SENSE2_PIN_LED, COLOR_ORDER>(sense2Leds, SENSE2_NUM_LEDS); 
    FastLED.addLeds<LED_TYPE, SENSE3_PIN_LED, COLOR_ORDER>(sense3Leds, SENSE3_NUM_LEDS); 
    FastLED.addLeds<LED_TYPE, SENSE4_PIN_LED, COLOR_ORDER>(sense4Leds, SENSE4_NUM_LEDS); 
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

#if DEBUG_SERIAL
void debugSerial(long start, long total)
{
    // check on performance in milliseconds
    Serial.print(millis() - start);
    Serial.print("\t");
    // print sensor output 1
    Serial.println(total);
}
#endif

void loop()
{
    long start = millis();
    long sense1 =  sense1Sensor.capacitiveSensor(30);
    long sense2 =  sense1Sensor.capacitiveSensor(30);
    long sense3 =  sense1Sensor.capacitiveSensor(30);
    long sense4 =  sense1Sensor.capacitiveSensor(30);

#if DEBUG_SERIAL
    debugSerial("sense1: ", start, sense1);
    debugSerial("sense2: ", start, sense2);
    debugSerial("sense3: ", start, sense3);
    debugSerial("sense4: ", start, sense4);
#endif

    if (sense1 > 50) {
        sense1Leds[0] = CRGB::White; FastLED.show(); 
    } else {
        sense1Leds[0] = CRGB::Black; FastLED.show();
    }

    if (sense2 > 50) {
        sense2Leds[0] = CRGB::White; FastLED.show(); 
    } else {
        sense2Leds[0] = CRGB::Black; FastLED.show();
    }

    if (sense3 > 50) {
        sense3Leds[0] = CRGB::White; FastLED.show(); 
    } else {
        sense3Leds[0] = CRGB::Black; FastLED.show();
    }

    if (sense4 > 50) {
        sense4Leds[0] = CRGB::White; FastLED.show(); 
    } else {
        sense4Leds[0] = CRGB::Black; FastLED.show();
    }
}

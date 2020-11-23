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
#define MAX_NUM_LEDS    60
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

#define SENSE_CYCLES 30

struct TouchLedStrip {
    const uint8_t sendPin;
    const uint8_t sensePin;
    const uint8_t ledPin;
    const uint8_t numLeds;

    CapacitiveSensor sensor;
    CRGB             leds[MAX_NUM_LEDS];

    TouchLedStrip(int sendPin, int sensePin, int ledPin, int numLeds) : sendPin(sendPin), sensePin(sensePin), ledPin(ledPin),numLeds(numLeds), sensor(sendPin,sensePin) { }

    void doSetup() {
        sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    }

    long sense() {
        return sensor.capacitiveSensor(SENSE_CYCLES);
    }
};

TouchLedStrip touchLedStrips[]{
    TouchLedStrip(PIN_TOUCH_SEND, SENSE1_PIN_RECEIVE, SENSE1_PIN_LED, SENSE1_NUM_LEDS),
    TouchLedStrip(PIN_TOUCH_SEND, SENSE2_PIN_RECEIVE, SENSE2_PIN_LED, SENSE2_NUM_LEDS), 
    TouchLedStrip(PIN_TOUCH_SEND, SENSE3_PIN_RECEIVE, SENSE3_PIN_LED, SENSE3_NUM_LEDS), 
    TouchLedStrip(PIN_TOUCH_SEND, SENSE4_PIN_RECEIVE, SENSE4_PIN_LED, SENSE4_NUM_LEDS) };


void setup()
{
#if DEBUG_SERIAL
    Serial.begin(9600);
#endif

    for (int index = 0; index < sizeof(touchLedStrips)/sizeof(TouchLedStrip); index++) {
        touchLedStrips[index].doSetup();
        // Workaround for the template
        switch(touchLedStrips[index].ledPin) {
            case SENSE1_PIN_LED:
                FastLED.addLeds<LED_TYPE, SENSE1_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                break;
            case SENSE2_PIN_LED:
                FastLED.addLeds<LED_TYPE, SENSE2_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                break;
            case SENSE3_PIN_LED:
                FastLED.addLeds<LED_TYPE, SENSE3_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                break;
            case SENSE4_PIN_LED:
                FastLED.addLeds<LED_TYPE, SENSE4_PIN_LED, COLOR_ORDER>(touchLedStrips[index].leds, touchLedStrips[index].numLeds); 
                break;
        }
    }
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

void loop()
{
    long start = millis();

    for (int index = 0; index < sizeof(touchLedStrips)/sizeof(TouchLedStrip); index++) {
        long sense = touchLedStrips[index].sense();
#if DEBUG_SERIAL
        debugSerial("senseTBD: ", start, sense1);
#endif

        if (sense > 50) {
            touchLedStrips[index].leds[0] = CRGB::White; FastLED.show(); 
        } else {
            touchLedStrips[index].leds[0] = CRGB::Black; FastLED.show();
        }
    }
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

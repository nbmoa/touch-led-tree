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

////////////////////////////////////////////////
// TOUCH TREE config
////////////////////////////////////////////////

#define TOUCH_TREE_NUM_STRIPS 4

#define TOUCH_TREE_LED_TYPE WS2812B             // WS2812B LED strips are used
#define TOUCH_TREE_COLOR_ORDER GRB              // WS2812B LED strips have GRB color order
#define TOUCH_TREE_DEFAULT_BRIGHTNESS 64        // TBD what is the range is 64 max?

#define TOUCH_TREE_CYCLE_INTERVAL 40 // 41.666 == 24 update / sec

// TBD this should be nicer
#define TOUCH_TREE_MAX_NUM_LEDS    60
#define TOUCH_TREE_SENSE1_NUM_LEDS 60
#define TOUCH_TREE_SENSE2_NUM_LEDS 60
#define TOUCH_TREE_SENSE3_NUM_LEDS 60
#define TOUCH_TREE_SENSE4_NUM_LEDS 60

#define TOUCH_TREE_PIN_TOUCH_SEND     4
#define TOUCH_TREE_SENSE1_PIN_RECEIVE 5
#define TOUCH_TREE_SENSE1_PIN_LED     6
#define TOUCH_TREE_SENSE2_PIN_RECEIVE 7
#define TOUCH_TREE_SENSE2_PIN_LED     8
#define TOUCH_TREE_SENSE3_PIN_RECEIVE 9
#define TOUCH_TREE_SENSE3_PIN_LED     10
#define TOUCH_TREE_SENSE4_PIN_RECEIVE 10
#define TOUCH_TREE_SENSE4_PIN_LED     11

#define TOUCH_TREE_DEFAULT_ACTIVE_COLOR    CHSV(0, 0, 255) // WHITE
#define TOUCH_TREE_DEFAULT_INACTIVE_COLOR  CHSV(0, 0,  30) // DIMMED WHITE

#define SENSE_MEASUREMENT_CYCLES 10

#define SENSE_STORED_PER_LED 10000
#define SENSE_STORED_PER_LED_100 100
#define DECREASE_SENSE_SPEED 400
#define INCREASE_SENSE_SPEED 200

#define ENABLE_RETRY_INTERVAL 1000 // if sense for a strip is disabled check every 1000 ms if the sense still times out
#define SENSE_DISABLE_TIMEOUT 2 // timeout for the measurement that disables the sense measurement for ENABLE_RETRY_INTERVAL
#define SENSE_ON_THREASHOLD 1000

#define TRANSPARENT_RUNNER_COLOR CHSV{0,0,0}
#define MAX_LEDS_PER_STRIP    60
#define MIN_RUNNER_START_INTERVAL_MS 200
#define MAX_ACTIVE_RUNNERS 4

// lastCycle stores the end of the last cycle, so we can wait
// wait that the next cycle will begin in CYCLE_INTERVAL 
long lastCycle = millis();

CRGB fade(CRGB from, CRGB to, int percent) {
    CRGB result;
    result.r = from.r + ((to.r - from.r) / 100) * percent;
    result.g = from.g + ((to.g - from.g) / 100) * percent;
    result.b = from.b + ((to.b - from.b) / 100) * percent;
    return result;
}

CHSV fade(CHSV from, CHSV to, int percent) {
    CHSV result;
    result.h = from.h + ((to.h - from.h) / 100) * percent;
    result.s = from.s + ((to.s - from.s) / 100) * percent;
    result.v = from.v + ((to.v - from.v) / 100) * percent;
    return result;
}

CHSV draw(CHSV back, CHSV frame) {
    CHSV result;
    uint8_t opacity = frame.v;
    result.h = back.h + ((frame.h - back.h) / 255) * opacity;
    result.s = back.s + ((frame.s - back.s) / 255) * opacity;
    result.v = back.v + ((255 - back.s) / 255) * opacity;
    return result;
}

struct SenseSensor {
    const uint8_t sendPin;
    const uint8_t sensePin;
    bool active;
    long disabledSince;
    long *cycleTimestamp;

    CapacitiveSensor sensor;

    SenseSensor(const uint8_t sendPin, const uint8_t sensePin, long *cycleTimestamp): sendPin(sendPin), sensePin(sensePin), active(false), sensor(sendPin,sendPin), cycleTimestamp(cycleTimestamp) {}
    void doSetup() { 
        sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    }
    bool sense() { 
        if (active) {
            long startTime = *cycleTimestamp;
            long senseVal = sensor.capacitiveSensor(SENSE_MEASUREMENT_CYCLES);
            if ( (*cycleTimestamp - startTime ) > SENSE_DISABLE_TIMEOUT) {
                active = false;
                disabledSince = startTime;
            }
            return senseVal >= SENSE_ON_THREASHOLD || senseVal <= -SENSE_ON_THREASHOLD;
        } else {
            if ( *cycleTimestamp > disabledSince + ENABLE_RETRY_INTERVAL ) {
                // This would also be a good place to recalibrate
                active = true;
            }
            return active; // for disabled sensors this will create pulses in the SENSE_DISABLE_TIMEOUT interval, should create nice runner effects if the sense is offline
        }
    }
};

struct LedRunner {
    bool active;
    int numLeds;
    int activeLed;
    long runnerSpeed;
    CHSV runnerColor;
    long glowTime;
    long *cycleTimestamp;

    uint8_t hueChange;     // amout of hue change in on interval
    long hueChangeInterval; // interval ms the change of the hue value is applied
    long hueLastChanged;

    long startTime;

    void init(int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime, long *cycleTimestamp) {
        numLeds = numLeds;
        runnerSpeed = runnerSpeed;
        runnerColor = runnerColor;
        hueChange = hueChange;
        hueChangeInterval = hueChangeInterval;
        glowTime = glowTime;
        cycleTimestamp = cycleTimestamp;
    }

    void updateHue() {
        long curTime = millis();
        if (hueLastChanged + hueChangeInterval > curTime) {
            hueChangeInterval = curTime;
            runnerColor + hueChange;
        }
    }
    void setActiveLed() {
        activeLed = numLeds * runnerSpeed / ( millis() - startTime ); 
    }
    CHSV ledColor(int ledIndex) {
        if ( activeLed < ledIndex ) {
            return TRANSPARENT_RUNNER_COLOR;
        }
    }
};

struct Background{
    int numLeds;         // Number of leds of the strip
    CHSV activeColor;    // LED active background color
    CHSV inactiveColor;  // LED inactive background color
    int ledActiveOffset; // the offset of the active led

    long *cycleTimestamp;

    Background(int numLeds, CHSV activeColor, CHSV inactiveColor, int ledActiveOffset, long *cycleTimestamp): numLeds(numLeds), activeColor(activeColor), inactiveColor(inactiveColor), ledActiveOffset(ledActiveOffset), cycleTimestamp(cycleTimestamp) {}
    void calculateCycle(bool sensed) {}

    CHSV getLedColor(int ledIndex) { }
};

struct RunnerCluster {
    LedRunner runner[MAX_ACTIVE_RUNNERS];

    CHSV getLedSprite(int ledIndex) { }
};

struct ImprovedTouchStrip {
    int numLeds;         // Number of leds of the strip
    CHSV runnerColor;    // LED color of the runner
    
    CRGB leds[MAX_LEDS_PER_STRIP];

    long baseRunnerTime;
    long runnerTimeDiff;

    SenseSensor sensor;
    Background backColor;
    bool previousSense;

    long curSense;           // stores the current sense value of the led strip
    long cycleTimestamp; // stores the timestamp of the last cycle

    long allActiveTimeMs;   // time it takes till all leds are lit
    
    long incSenseMsRatio;  // ratio the cycle duration in ms multiplied with if sense was active in that cycle
    long decSenseMsRatio;  // ratio the cycle duration in ms multiplied with if sense was not active in that cycle
    
    long allActiveSense;  // the sense value of the strip to light up all light, this is calculated baed on the allActiveTime
    long maxSense;        // the max sense value the strip can hold
    
    long lastRunnerStartTime;

    ImprovedTouchStrip(uint8_t sendPin, uint8_t sensePin, int numLeds, CHSV activeColor, CHSV inactiveColor, int ledActiveOffset, long allActiveTimeMs, long incSenseMsRatio, long decSenseMsRatio, long maxSenseOffset)
        : numLeds(numLeds),
          backColor(numLeds, activeColor, inactiveColor, ledActiveOffset, &cycleTimestamp), 
          sensor(sendPin, sensePin, &cycleTimestamp),
          curSense(0),
          cycleTimestamp(millis()),
          allActiveTimeMs(allActiveTimeMs), 
          incSenseMsRatio(incSenseMsRatio), 
          decSenseMsRatio(decSenseMsRatio), 
          allActiveSense(allActiveTimeMs * incSenseMsRatio), 
          maxSense(allActiveTimeMs * incSenseMsRatio + maxSenseOffset) { }

    void runCycle(){
        // First update the timestamp
        long lastCycleTimestamp = cycleTimestamp;
        cycleTimestamp = millis();

        // Read in the sensor
        bool sensed =  sensor.sense();
        backColor.calculateCycle(sensed);
        if ( sensed == true && previousSense == false ) {
            //try to start a new runner
            if ( cycleTimestamp - lastRunnerStartTime > MIN_RUNNER_START_INTERVAL_MS ) {
                tryStartRunner();                
            }
        }
        for ( int i = 0; i < numLeds; i++ ) {
            CHSV ledBackColor = backColor.getLedColor(i);
            // MOA TBD renable again the runner
            CHSV ledSprite = backColor.getLedColor(i);
            //CHSV ledSprite = runners.getLedSprite(i);
            leds[i] = draw(ledBackColor, ledSprite); 
        }
    }
    void tryStartRunner() {}
    CHSV plotRunner(int ledIndex) {}
    void doSetup() {}
};

struct TouchTree {
    long currentTimestamp;
    long lastCycleTimestamp;

    ImprovedTouchStrip improvedTouchLeds[TOUCH_TREE_NUM_STRIPS];
    RunnerCluster      runnerCluster;
    
    TouchTree()
        :improvedTouchLeds({
            ImprovedTouchStrip(TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE1_PIN_RECEIVE, TOUCH_TREE_SENSE1_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
            ImprovedTouchStrip(TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE2_PIN_RECEIVE, TOUCH_TREE_SENSE2_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
            ImprovedTouchStrip(TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE3_PIN_RECEIVE, TOUCH_TREE_SENSE3_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
            ImprovedTouchStrip(TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE4_PIN_RECEIVE, TOUCH_TREE_SENSE4_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
        }),
        runnerCluster() {
    }

    void setup() {
        for (int index = 0; index < sizeof(improvedTouchLeds)/sizeof(ImprovedTouchStrip); index++) {
            improvedTouchLeds[index].doSetup();
            // setup the led pins that control the led strips, based on the sense led pin
            switch(improvedTouchLeds[index].sensor.sensePin) {
                case TOUCH_TREE_SENSE1_PIN_RECEIVE:
                    FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE1_PIN_LED, TOUCH_TREE_COLOR_ORDER>(improvedTouchLeds[index].leds, improvedTouchLeds[index].numLeds); 
                    break;
                case TOUCH_TREE_SENSE2_PIN_RECEIVE:
                    FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE2_PIN_LED, TOUCH_TREE_COLOR_ORDER>(improvedTouchLeds[index].leds, improvedTouchLeds[index].numLeds); 
                    break;
                case TOUCH_TREE_SENSE3_PIN_RECEIVE:
                    FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE3_PIN_LED, TOUCH_TREE_COLOR_ORDER>(improvedTouchLeds[index].leds, improvedTouchLeds[index].numLeds); 
                    break;
                case TOUCH_TREE_SENSE4_PIN_RECEIVE:
                    FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE4_PIN_LED, TOUCH_TREE_COLOR_ORDER>(improvedTouchLeds[index].leds, improvedTouchLeds[index].numLeds); 
                    break;
                default:
                    DEBUG_PRINTLN("ERROR: led pin strip unknown touch pin");
            }
        }

        // TBD not sure about this
        FastLED.setBrightness(TOUCH_TREE_DEFAULT_BRIGHTNESS);
    }

    void loop() {
        currentTimestamp = millis();
        for (int index = 0; index < TOUCH_TREE_NUM_STRIPS; index++) {
            improvedTouchLeds[index].runCycle();
        }
       
        FastLED.show();
        
        long timeRemain = (lastCycleTimestamp + TOUCH_TREE_CYCLE_INTERVAL) - millis();
        if (timeRemain > 0) {
            delay(timeRemain);
        }

        DEBUG_PRINT("timeRemain: ");
        DEBUG_PRINTDEC(timeRemain);
        DEBUG_PRINTLN("");
        lastCycleTimestamp = millis();
    };
};

TouchTree touchTree;

void setup()
{
#if DEBUG
    Serial.begin(9600);
#endif
    touchTree.setup();
}

void loop()
{
    touchTree.loop();
}


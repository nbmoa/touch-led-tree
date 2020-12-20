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

#define SENSE_STORED_PER_LED 10000
#define SENSE_STORED_PER_LED_100 100
#define DECREASE_SENSE_SPEED 400
#define INCREASE_SENSE_SPEED 200

#define MAX_LEDS_PER_STRIP    60
#define LED_LEAF_MIN_RUNNER_START_INTERVAL_MS 200
#define RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS 4

long cycleTimestamp;

//CRGB fade(CRGB from, CRGB to, int percent) {
//    CRGB result;
//    result.r = from.r + ((to.r - from.r) / 100) * percent;
//    result.g = from.g + ((to.g - from.g) / 100) * percent;
//    result.b = from.b + ((to.b - from.b) / 100) * percent;
//    return result;
//}
//
//CHSV fade(CHSV from, CHSV to, int percent) {
//    CHSV result;
//    result.h = from.h + ((to.h - from.h) / 100) * percent;
//    result.s = from.s + ((to.s - from.s) / 100) * percent;
//    result.v = from.v + ((to.v - from.v) / 100) * percent;
//    return result;
//}

CHSV overlaySprites(CHSV s1, CHSV s2) {
    CHSV result;
    if ( s1.v == 0 ) {
        return s2;
    }
    uint8_t v = s1.v;
    if ( s2.v > s1.v ) {
        v = s2.v;
    }
    uint8_t s = s1.s;
    if ( s2.s > s1.s ) {
        s = s2.s;
    }
    // TBD Not sure how this will look like, need to play around with it
    result.h = s1.h + s2.h;
    result.s = s;
    result.v = v;
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


#define SENSE_SENSOR_MEASUREMENT_SAMPLES 10
#define SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS 1000 // if sense for a strip is disabled check every 1000 ms if the sense still times out
#define SENSE_SENSOR_DISABLE_TIMEOUT_MS 2 // timeout for the measurement that disables the sense measurement for SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS
#define SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD 1000
struct SenseSensor {
    const uint8_t sensePin;
    bool active;
    long disabledSince;

    CapacitiveSensor sensor;

    SenseSensor(const uint8_t sendPin, const uint8_t sensePin): sensePin(sensePin), active(false), disabledSince(0), sensor(sendPin,sensePin) {}
    void doSetup() { 
        sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    }
    bool sense() { 
        if (active) {
            long startTime = cycleTimestamp;
            long senseVal = sensor.capacitiveSensor(SENSE_SENSOR_MEASUREMENT_SAMPLES);
            if ( (cycleTimestamp - startTime ) > SENSE_SENSOR_DISABLE_TIMEOUT_MS) {
                active = false;
                disabledSince = startTime;
            }
            return senseVal >= SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD || senseVal <= -SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD;
        } else {
            if ( cycleTimestamp > disabledSince + SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS ) {
                // This would also be a good place to recalibrate
                active = true;
            }
            return active; // for disabled sensors this will create pulses in the SENSE_SENSOR_DISABLE_TIMEOUT_MS interval, should create nice runner effects if the sense is offline
        }
    }
};


#define HSV_COLOR_TRANSPARENT CHSV{0,0,0}
struct LedRunner {
    bool active;
    
    uint8_t stripID;
    int numLeds;
    long runnerSpeed;
    CHSV runnerColor;
    long glowTime;
    uint8_t hueChange;     // amout of hue change in on interval
    long hueChangeInterval; // interval ms the change of the hue value is applied
    
    int activeLed;
    long hueLastChanged;
    long startTime;

    void start(uint8_t stripID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime) {
        active = true;
        stripID = stripID;
        numLeds = numLeds;
        runnerSpeed = runnerSpeed;
        runnerColor = runnerColor;
        glowTime = glowTime;
        hueChange = hueChange;
        hueChangeInterval = hueChangeInterval;
        hueLastChanged = cycleTimestamp;
        startTime = cycleTimestamp;
    }

    void updateHue() {
        if (hueLastChanged + hueChangeInterval > cycleTimestamp) {
            hueChangeInterval = cycleTimestamp;
            runnerColor.h + hueChange;
        }
    }
    void updateActiveLed() {
        activeLed = numLeds * runnerSpeed / ( cycleTimestamp - startTime ); 
    }
    void updateRunner() {
        updateActiveLed();
        updateHue();
    }
    CHSV ledColor(int ledIndex) {
        if ( ledIndex > activeLed ) {
            return HSV_COLOR_TRANSPARENT;
        }
        if ( ledIndex < activeLed ) {
            if ( glowTime == 0 ) {
                return HSV_COLOR_TRANSPARENT;
            }
            long activeLedTimestamp = runnerSpeed * ledIndex + startTime;
            if ( activeLedTimestamp + glowTime > cycleTimestamp ) {
                uint8_t transValue = 255 * (activeLedTimestamp + glowTime - cycleTimestamp) / glowTime;
                return CHSV(runnerColor.h, 255, transValue);
            }
            return HSV_COLOR_TRANSPARENT;
        }
        return runnerColor;
    }
};

struct RunnerCluster {
    LedRunner runner[RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS];

    void triggerRunner(uint8_t stripID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime) {
        for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
            if ( !runner[i].active ) {
                runner[i].start(stripID, numLeds, runnerSpeed, runnerColor, hueChange, hueChangeInterval, glowTime);
                return;
            }
        }
        DEBUG_PRINTLN("failed to trigger new runner");
    }
    CHSV getLedSprite(uint8_t stripID, int ledIndex) {
        CHSV ledSprite = HSV_COLOR_TRANSPARENT;
        for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
            if ( runner[i].active && ( runner[i].stripID == stripID ) ) {
                ledSprite = overlaySprites(ledSprite, runner[i].ledColor(ledIndex));
            }
        }
        return ledSprite;
    }
    void update() {
        for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
            if ( runner[i].active == true ) {
                runner[i].updateRunner();
            }
        }
    }
};

struct SenseFilling {
    int numLeds;         // Number of leds of the strip
    CHSV activeColor;    // LED active background color
    CHSV inactiveColor;  // LED inactive background color
    int ledActiveOffset; // the offset of the active led
    uint8_t incSenseMsRatio;
    uint8_t decSenseMsRatio;

    long accumulatedSense;
    long lastSenseTimestamp;

    SenseFilling(int numLeds, CHSV activeColor, CHSV inactiveColor, int ledActiveOffset, uint8_t incSenseMsRatio, uint8_t decSenseMsRatio): numLeds(numLeds), activeColor(activeColor), inactiveColor(inactiveColor), ledActiveOffset(ledActiveOffset), lastSenseTimestamp(millis()), incSenseMsRatio(incSenseMsRatio), decSenseMsRatio(decSenseMsRatio) {}
    void calculateCycle(bool sensed) {
        if (sensed) {
            accumulatedSense = accumulatedSense + ( cycleTimestamp - lastSenseTimestamp ) * incSenseMsRatio;
        } else {
            if ( accumulatedSense < 1 ) {
                accumulatedSense = accumulatedSense - ( cycleTimestamp - lastSenseTimestamp ) * decSenseMsRatio;
                if ( accumulatedSense < 0 ) { 
                    accumulatedSense = 0 ;
                }
            }
        }
    }
    long getCurrentSense() {
        return accumulatedSense;
    }
    CHSV getLedBackColor(int ledIndex) { }
};

struct Background {
    CHSV getLedBackColor(int ledIndex, long senseValue) {
    }
};

///////////////////////////////////////////////////////////
// LedLeaf
///////////////////////////////////////////////////////////

#define LED_LEAF_DEFAULT_RUNNER_SPEED_MS 10000 
#define LED_LEAF_DEFAULT_RUNNER_COLOR CHSV(32,255,255)
#define LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE 1
#define LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS 40
#define LED_LEAF_DEFAULT_RUNNER_GLOWTIME_MS 3000
struct LedLeaf {
    uint8_t stripID;
    int numLeds;         // Number of leds of the strip
    CHSV runnerColor;    // LED color of the runner

    CRGB leds[MAX_LEDS_PER_STRIP];

    long baseRunnerTime;
    long runnerTimeDiff;

    SenseSensor     sensor;
    SenseFilling    senseFilling;
    RunnerCluster   *runnerCluster;
    Background      background;
    bool previousSense;

    long curSense;           // stores the current sense value of the led strip

    long allActiveTimeMs;   // time it takes till all leds are lit
    
    long incSenseMsRatio;  // ratio the cycle duration in ms multiplied with if sense was active in that cycle
    long decSenseMsRatio;  // ratio the cycle duration in ms multiplied with if sense was not active in that cycle
    
    long allActiveSense;  // the sense value of the strip to light up all light, this is calculated baed on the allActiveTime
    long maxSense;        // the max sense value the strip can hold
    
    long lastRunnerStartTime;

    LedLeaf(RunnerCluster *runnerCluster,uint8_t stripID, uint8_t sendPin, uint8_t sensePin, int numLeds, CHSV activeColor, CHSV inactiveColor, int ledActiveOffset, long allActiveTimeMs, uint8_t incSenseMsRatio, uint8_t decSenseMsRatio, long maxSenseOffset);
    void runCycle();
    void doSetup();
};
    
LedLeaf::LedLeaf(RunnerCluster *runnerCluster,uint8_t stripID, uint8_t sendPin, uint8_t sensePin, int numLeds, CHSV activeColor, CHSV inactiveColor, int ledActiveOffset, long allActiveTimeMs, uint8_t incSenseMsRatio, uint8_t decSenseMsRatio, long maxSenseOffset)
    : stripID(stripID),
      runnerCluster(runnerCluster),
      numLeds(numLeds),
      senseFilling(numLeds, activeColor, inactiveColor, ledActiveOffset, incSenseMsRatio, decSenseMsRatio), 
      sensor(sendPin, sensePin),
      curSense(0),
      allActiveTimeMs(allActiveTimeMs), 
      incSenseMsRatio(incSenseMsRatio), 
      decSenseMsRatio(decSenseMsRatio), 
      allActiveSense(allActiveTimeMs * incSenseMsRatio), 
      maxSense(allActiveTimeMs * incSenseMsRatio + maxSenseOffset) { }

void LedLeaf::runCycle(){
    // Read in the sensor
    bool sensed =  sensor.sense();
    senseFilling.calculateCycle(sensed);
    if ( sensed == true && previousSense == false ) {
        //try to start a new runner
        if ( cycleTimestamp - lastRunnerStartTime > LED_LEAF_MIN_RUNNER_START_INTERVAL_MS ) {
            runnerCluster->triggerRunner(stripID, numLeds, LED_LEAF_DEFAULT_RUNNER_SPEED_MS, LED_LEAF_DEFAULT_RUNNER_COLOR, LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE, LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS, LED_LEAF_DEFAULT_RUNNER_GLOWTIME_MS);                
        }
    }
    for ( int i = 0; i < numLeds; i++ ) {
        CHSV ledBackColor = background.getLedBackColor(i, senseFilling.getCurrentSense());
        CHSV ledSprite = runnerCluster->getLedSprite(stripID, i);
        leds[i] = draw(ledBackColor, ledSprite); 
    }
}
void LedLeaf::doSetup() {}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// TouchTree
///////////////////////////////////////////////////////////

struct TouchTree {
    long currentTimestamp;
    long lastCycleTimestamp;

    RunnerCluster      runnerCluster;
    LedLeaf ledLeaf[TOUCH_TREE_NUM_STRIPS];
    
    TouchTree();
    void setup();
    void loop(); 
};
    
TouchTree::TouchTree()
    :ledLeaf({
        LedLeaf(&this->runnerCluster, 1, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE1_PIN_RECEIVE, TOUCH_TREE_SENSE1_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
        LedLeaf(&this->runnerCluster, 2, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE2_PIN_RECEIVE, TOUCH_TREE_SENSE2_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
        LedLeaf(&this->runnerCluster, 3, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE3_PIN_RECEIVE, TOUCH_TREE_SENSE3_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
        LedLeaf(&this->runnerCluster, 4, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE4_PIN_RECEIVE, TOUCH_TREE_SENSE4_NUM_LEDS, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2, 4000, 4, 1, 0),
    }),
    runnerCluster() {
}

void TouchTree::setup() {
    for (int index = 0; index < TOUCH_TREE_NUM_STRIPS; index++) {
        ledLeaf[index].doSetup();
        // setup the led pins that control the led strips, based on the sense led pin
        switch(ledLeaf[index].sensor.sensePin) {
            case TOUCH_TREE_SENSE1_PIN_RECEIVE:
                FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE1_PIN_LED, TOUCH_TREE_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            case TOUCH_TREE_SENSE2_PIN_RECEIVE:
                FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE2_PIN_LED, TOUCH_TREE_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            case TOUCH_TREE_SENSE3_PIN_RECEIVE:
                FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE3_PIN_LED, TOUCH_TREE_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            case TOUCH_TREE_SENSE4_PIN_RECEIVE:
                FastLED.addLeds<TOUCH_TREE_LED_TYPE, TOUCH_TREE_SENSE4_PIN_LED, TOUCH_TREE_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            default:
                DEBUG_PRINTLN("ERROR: led pin strip unknown touch pin");
        }
    }

    // TBD not sure about this
    FastLED.setBrightness(TOUCH_TREE_DEFAULT_BRIGHTNESS);
}

void TouchTree::loop() {
    // First update the timestamp
    long lastCycleTimestamp = cycleTimestamp;
    cycleTimestamp = millis();

    // Update the runners
    runnerCluster.update();

    for (int index = 0; index < TOUCH_TREE_NUM_STRIPS; index++) {
        ledLeaf[index].runCycle();
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

///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// Main defines
///////////////////////////////////////////////////////////

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


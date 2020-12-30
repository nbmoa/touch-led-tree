
//#define DEBUG 1
#define ENABLE_DEBUG_PRINTS 1
//#define DEBUG_CYCLES 1
//#define DEBUG_SENSOR 1
//#define DEBUG_SENSOR_RAW_SENSE 1
#define DEBUG_RUNNER 1
//#define DEBUG_RUNNER_GLOW 1
//#define DEBUG_RUNNER_ACTIVE_LED 1
#define DEBUG_SPRITE 1

#define HSV_COLOR_WHITE       CHSV(0, 0, 255)
#define HSV_COLOR_DIMM_WHITE  CHSV(0, 0, 30)

#define HSV_COLOR_TRANSPARENT CHSV{0,0,0}

#define CONFIG_SENSE_ACTIVE_THREASHOLD        5000
#define CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS 10000
#define CONFIG_MAX_LEDS_PER_STRIP 12
#define CONFIG_SENSE1_NUM_LEDS    12
#define CONFIG_SENSE2_NUM_LEDS    12
#define CONFIG_SENSE3_NUM_LEDS    12
#define CONFIG_SENSE4_NUM_LEDS    12

#define CONFIG_ACTIVE_COLOR    HSV_COLOR_WHITE
#define CONFIG_INACTIVE_COLOR  HSV_COLOR_WHITE

#define CONFIG_MAX_ACTIVE_RUNNERS 16
#define CONFIG_RUNNER_GLOW_NUM_LEDS 4
#define CONFIG_RUNNER_COLOR       CHSV(64,255,255)
#define CONFIG_RUNNER_HUE_CHANGE  20
#define CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS 200

// PIN configuration
#define CONFIG_PIN_TOUCH_SEND     2
#define CONFIG_SENSE1_PIN_RECEIVE 3
#define CONFIG_SENSE1_PIN_LED     7
#define CONFIG_SENSE2_PIN_RECEIVE 4
#define CONFIG_SENSE2_PIN_LED     8
#define CONFIG_SENSE3_PIN_RECEIVE 5
#define CONFIG_SENSE3_PIN_LED     9
#define CONFIG_SENSE4_PIN_RECEIVE 6
#define CONFIG_SENSE4_PIN_LED     10

#include <CapacitiveSensor.h>
// This suppress the pragma warning of FastLED (see https://github.com/FastLED/FastLED/issues/797)
#define FASTLED_INTERNAL
#include "FastLED.h"

#include "debug.h"
#include "sense_sensor.h"
#include "led_runner.h"
#include "runner_cluster.h"
#include "background.h"
#include "stored_time.h"
#include "led_leaf.h"
#include "touch_tree.h"

///////////////////////////////////////////////////////////
// Main defines
///////////////////////////////////////////////////////////
// MOA OK

// cycle timestamp (global var because its used in many places
long cycleTimestamp;

// defined globaly so it will be a static struct
TouchTree touchTree;

void setup()
{
#if DEBUG
    Serial.begin(9600);
#endif
    PRINTLN("INFO: startup tree");
    touchTree.setup();
}

void loop()
{
    touchTree.loop();
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// TouchTree
///////////////////////////////////////////////////////////
// MOA TBD check, but looks good
    
TouchTree::TouchTree()
    :ledLeaf({
        LedLeaf(1, TOUCH_TREE_SENSE1_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE1_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 1000, 1000, 0, 1, 1, CHSV(0,255,255), 1000, 4000, &this->runnerCluster),
        LedLeaf(2, TOUCH_TREE_SENSE2_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE2_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 1000, 1000, 0, 2, 2, CHSV(64,255,255), 1000, 4000, &this->runnerCluster),
        LedLeaf(3, TOUCH_TREE_SENSE3_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE3_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 1000, 1000, 0, 1, 2, CHSV(128,255,255), 1000, 4000, &this->runnerCluster),
        LedLeaf(4, TOUCH_TREE_SENSE4_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE4_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 1000, 1000, 0, 4, 4, CHSV(192,255,255), 1000, 4000, &this->runnerCluster),
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
                PRINTLN("ERROR: led pin strip unknown touch pin");
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

#ifdef DEBUG_CYCLES
    DEBUG_PRINTDECLN("cycle-time-diff: ", timeRemain);
#endif
};
// MOA TBD check

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// LedLeaf
///////////////////////////////////////////////////////////
    
LedLeaf::LedLeaf(uint8_t leafID,
                 int     numLeds,
                 uint8_t sendPin,
                 uint8_t sensePin,
                 CHSV    backgroundActiveColor,
                 CHSV    backgroundInactiveColor,
                 long    timePerLed,
                 long    timeMinStored,
                 long    timeMaxStoredOffset,
                 uint8_t timeIncRatio,
                 uint8_t timeDecRatio,
                 CHSV    runnerColor,
                 long    runnerBaseTime,
                 long    runnerDiffTime,
                 RunnerCluster *runnerCluster)
    : leafID(leafID),
      numLeds(numLeds),
      runnerColor(runnerColor),
      runnerBaseTime(runnerBaseTime),
      runnerDiffTime(runnerDiffTime),
      previousSenseState(false),
      lastRunnerStartTime(0),
      runnerCluster(runnerCluster),
      sensor(sendPin, sensePin),
      storedTime(timeIncRatio, timeDecRatio, timeMinStored, (numLeds * timePerLed) + timeMaxStoredOffset),
      background(backgroundActiveColor, backgroundInactiveColor, timePerLed) {
}

void LedLeaf::runCycle(){
    // Read in the sensor

    bool sensed =  sensor.sense();
#ifdef DEBUG_SENSOR
    DEBUG_PRINT("leafID: ");
    DEBUG_PRINTDEC(leafID);
    if ( sensed ) {
      DEBUG_PRINTDECLN(", sense: ON,  time: ", storedTime.storedTime);
    } else {
      DEBUG_PRINTDECLN(", sense: OFF, time: ", storedTime.storedTime);
    }
#endif

    // Update storedTime
    storedTime.update(sensed);

    // Handle new timers
    if ( sensed == true && previousSenseState == false ) {
        //try to start a new runner if LED_LEAF_MIN_RUNNER_START_INTERVAL_MS already elapsed since last started runner
        if ( cycleTimestamp - lastRunnerStartTime > LED_LEAF_MIN_RUNNER_START_INTERVAL_MS ) {
        //    runnerCluster->triggerRunner(leafID, numLeds, runnerBaseTime + random(runnerDiffTime), runnerColor, LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE, LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS, LED_LEAF_DEFAULT_RUNNER_GLOW_NUM_LEDS);                
            runnerCluster->triggerRunner(leafID, numLeds, runnerBaseTime + random(runnerDiffTime), CHSV(random(256),255,255), LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE, LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS, LED_LEAF_DEFAULT_RUNNER_GLOW_NUM_LEDS);                
        }
    }
    previousSenseState = sensed;

    // update the leds based on the stored time and the active runners of the leaf
    for ( int i = 0; i < numLeds; i++ ) {
        CHSV ledBackColor = background.getLedBackColor(i, storedTime.storedTime);
        CHSV ledSprite = runnerCluster->getLedSprite(leafID, i);

        CHSV ledValue = draw(ledBackColor, ledSprite);
#ifdef DEBUG_SPRITE
    if ( leafID == 2 && i == 1 ) {
        DEBUG_PRINT("sprite[");
        DEBUG_PRINTDEC(i);
        DEBUG_PRINT("] - h: ");
        DEBUG_PRINTDEC(ledValue.h);
        DEBUG_PRINT(", s: ");
        DEBUG_PRINTDEC(ledValue.s);
        DEBUG_PRINTDECLN(", v: ", ledValue.v);
    }
#endif

        leds[i] = ledValue;
    }
}

void LedLeaf::doSetup() {
    // led initialization is done in the TouchTree

    // setup the sensor
    sensor.doSetup();
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Background
///////////////////////////////////////////////////////////
// MOA OK

Background::Background(CHSV activeColor,
                       CHSV inactiveColor,
                       long sensePerLed)
    : activeColor(activeColor),
      inactiveColor(inactiveColor),
      sensePerLed(sensePerLed) {
}

CHSV Background::getLedBackColor(int ledIndex, long senseValue) {
    long ledSenseValue = ledIndex * sensePerLed;
    if ( ledSenseValue < senseValue ) {
        return activeColor; 
    }
    if ( ledSenseValue + sensePerLed >= senseValue ) {
        return inactiveColor;
    }
    int activePercent = (senseValue - ledSenseValue) / (100 * sensePerLed);
    return fade(inactiveColor, activeColor, activePercent);
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// StoredTime
///////////////////////////////////////////////////////////
// MOA OK

StoredTime::StoredTime(uint8_t incRatio, uint8_t decRatio, long minTime, long maxTime)
    : incRatio(incRatio),
      decRatio(decRatio),
      minTime(minTime),
      maxTime(maxTime),
      storedTime(minTime),
      lastCycleTimestamp(millis()) {}

void StoredTime::update(bool sensed) {
    if (sensed) {
        if ( storedTime < maxTime ) {
            storedTime = storedTime + ( cycleTimestamp - lastCycleTimestamp ) * incRatio;
            if ( storedTime > maxTime ) {
                storedTime = maxTime;
            }
        }
    } else {
        if ( storedTime > minTime ) {
            storedTime = storedTime - ( cycleTimestamp - lastCycleTimestamp ) * decRatio;
            if ( storedTime < minTime ) { 
                storedTime = minTime;
            }
        }
    }
    lastCycleTimestamp = cycleTimestamp;
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// RunnerCluster
///////////////////////////////////////////////////////////
// MOA checked

void RunnerCluster::triggerRunner(uint8_t leafID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, int glowNumLeds) {
    for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( !runner[i].active ) {
            runner[i].start(leafID, numLeds, runnerSpeed, runnerColor, hueChange, hueChangeInterval, glowNumLeds);
            return;
        }
    }
    PRINTLN("ERROR: failed to trigger new runner");
}

CHSV RunnerCluster::getLedSprite(uint8_t leafID, int ledIndex) {
    CHSV ledSprite = HSV_COLOR_TRANSPARENT;
    for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( runner[i].active && ( runner[i].leafID == leafID ) ) {
            ledSprite = overlaySprites(ledSprite, runner[i].ledColor(ledIndex));
        }
    }
    return ledSprite;
}

void RunnerCluster::update() {
    for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( runner[i].active == true ) {
            runner[i].updateRunner();
        }
    }
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// LedRunner
///////////////////////////////////////////////////////////
// MOA checked

void LedRunner::start(uint8_t leafID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, int glowNumLeds) {
    this->active = true;
    this->leafID = leafID;
    this->numLeds = numLeds;
    this->runnerSpeed = runnerSpeed;
    this->runnerLedSpeed = runnerSpeed / numLeds;
    this->runnerColor = runnerColor;
    this->glowNumLeds = glowNumLeds;
    this->hueChange = hueChange;
    this->hueChangeInterval = hueChangeInterval;
    this->hueLastChanged = cycleTimestamp;
    this->startTime = cycleTimestamp;
    updateActiveLed();

#ifdef DEBUG_RUNNER
    DEBUG_PRINT("runner - start, leafID: ");
    DEBUG_PRINTDEC(leafID);
    DEBUG_PRINT(", color: ");
    DEBUG_PRINTDEC(runnerColor.h);
    DEBUG_PRINTDECLN(", glowNumLeds: ", glowNumLeds);
#endif

}

void LedRunner::updateHue() {
    if ( cycleTimestamp > hueLastChanged + hueChangeInterval ) {
        this->hueLastChanged = cycleTimestamp;
        this->runnerColor.h += random(hueChange);
    }
}
void LedRunner::updateActiveLed() {
    activeLed =  ( numLeds * ( cycleTimestamp - startTime ) ) / runnerSpeed; 
#ifdef DEBUG_RUNNER_ACTIVE_LED
    DEBUG_PRINTDECLN("runner - active led: ", activeLed);
#endif
}
void LedRunner::updateRunner() {
    updateActiveLed();
    updateHue();
    if ( activeLed > (numLeds + glowNumLeds) ) {
#ifdef DEBUG_RUNNER
        DEBUG_PRINTLN("runner - finished");
#endif
        this->active = false;
    }
}
CHSV LedRunner::ledColor(int ledIndex) {
    if ( ledIndex > activeLed ) {
        return HSV_COLOR_TRANSPARENT;
    }
    if ( ledIndex < activeLed ) {
        if ( ledIndex > activeLed - glowNumLeds ) {
#ifdef DEBUG_RUNNER_GLOW
            DEBUG_PRINTDECLN("glowing led: ", ledIndex);
#endif
            uint8_t transValue = (glowNumLeds - (activeLed - ledIndex)) * 255 / glowNumLeds;
//#ifdef DEBUG_RUNNER_GLOW
//                DEBUG_PRINT("runner-color [");
//                DEBUG_PRINTDEC(ledIndex);
//                DEBUG_PRINT("]-h: ");
//                DEBUG_PRINTDEC(runnerColor.h);
//                DEBUG_PRINTDECLN(", tans: ", transValue);
//#endif
    

            return CHSV(runnerColor.h, 255, transValue);
        }
        return HSV_COLOR_TRANSPARENT;
    }

//    DEBUG_PRINTDECLN("MOADEBUG: color: ", runnerColor.h);

    return runnerColor;
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// SenseSensor
///////////////////////////////////////////////////////////
// MOA checked

SenseSensor::SenseSensor(const uint8_t sendPin, const uint8_t sensePin): sensePin(sensePin), active(false), disabledSince(-SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS), sensor(sendPin,sensePin) {
}

void SenseSensor::doSetup() { 
    sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    sensor.set_CS_Timeout_Millis(SENSE_SENSOR_DISABLE_TIMEOUT_MS + 1);
}

bool SenseSensor::sense() { 
    if (active) {
        long startTime = cycleTimestamp;
        long senseVal = sensor.capacitiveSensor(SENSE_SENSOR_MEASUREMENT_SAMPLES);
        if ( (millis() - startTime ) > SENSE_SENSOR_DISABLE_TIMEOUT_MS) {
            PRINTLN("WARN: disable sensor because of timeout");
            this->active = false;
            this->disabledSince = startTime;
        }
#ifdef DEBUG_SENSOR_RAW_SENSE
        DEBUG_PRINTDECLN("raw sense: ", senseVal);
#endif
        return senseVal >= SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD || senseVal <= -SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD;
    } else {
        if ( cycleTimestamp > disabledSince + SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS ) {
            PRINTLN("WARN: try to enable sensor because retry interval elapsed");
            // This would also be a good place to recalibrate
            this->active = true;
        }
        return active; // for disabled sensors this will create pulses in the SENSE_SENSOR_DISABLE_TIMEOUT_MS interval, should create nice runner effects if the sense is offline
    }
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////////////////

CHSV fade(CHSV from, CHSV to, int percent) {
    CHSV result;
    result.h = from.h + ((to.h - from.h) / 100) * percent;
    result.s = from.s + ((to.s - from.s) / 100) * percent;
    result.v = from.v + ((to.v - from.v) / 100) * percent;
    return result;
}

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
    if ( back.s == 0 ) {
      result.h = frame.h;
    }
    result.s = back.s + ((frame.s - back.s) / 255) * opacity;
    if ( back.v + opacity > 255 ) {
        result.v = 255;
    } else {
        result.v = back.v + opacity;
    }
    return result;
}

///////////////////////////////////////////////////////////

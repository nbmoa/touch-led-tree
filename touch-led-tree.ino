
//#define DEBUG 1

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

// cycle timestamp (global var because its used in many places
long cycleTimestamp;

// defined globaly so it will be a static struct
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

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// TouchTree
///////////////////////////////////////////////////////////
    
TouchTree::TouchTree()
    :ledLeaf({
        LedLeaf(1, TOUCH_TREE_SENSE1_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE1_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 1000, 2000, 10000, 1, 1, CHSV(32,255,255), 1000, 0, &this->runnerCluster),
        LedLeaf(2, TOUCH_TREE_SENSE2_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE2_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 2000, 2000, 20000, 1, 2, CHSV(64,255,255), 500, 0, &this->runnerCluster),
        LedLeaf(3, TOUCH_TREE_SENSE3_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE3_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 1500, 750, 10000, 2, 1, CHSV(96,255,255), 700, 0, &this->runnerCluster),
        LedLeaf(4, TOUCH_TREE_SENSE4_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE4_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 750, 1000, 10000, 1, 1, CHSV(128,255,255), 300, 0, &this->runnerCluster),
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

    DEBUG_PRINTDECLN("timeRemain: ", timeRemain);
    lastCycleTimestamp = millis();
};

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
      background(backgroundActiveColor, backgroundInactiveColor, timePerLed) {}

void LedLeaf::runCycle(){
    // Read in the sensor
    bool sensed =  sensor.sense();
    storedTime.runCycle(sensed);
    if ( sensed == true && previousSenseState == false ) {
        //try to start a new runner
        if ( cycleTimestamp - lastRunnerStartTime > LED_LEAF_MIN_RUNNER_START_INTERVAL_MS ) {
            runnerCluster->triggerRunner(leafID, numLeds, LED_LEAF_DEFAULT_RUNNER_SPEED_MS, LED_LEAF_DEFAULT_RUNNER_COLOR, LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE, LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS, LED_LEAF_DEFAULT_RUNNER_GLOWTIME_MS);                
        }
    }
    for ( int i = 0; i < numLeds; i++ ) {
        CHSV ledBackColor = background.getLedBackColor(i, storedTime.storedTime);
        CHSV ledSprite = runnerCluster->getLedSprite(leafID, i);
        leds[i] = draw(ledBackColor, ledSprite); 
    }
}

void LedLeaf::doSetup() {
// TBD
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Background
///////////////////////////////////////////////////////////
// MOA checked

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
        return inactiveColor; 
    }
    if ( ledSenseValue + sensePerLed >= senseValue ) {
        return activeColor;
    }
    int activePercent = (senseValue - ledSenseValue) / (100 * sensePerLed);
    return fade(inactiveColor, activeColor, activePercent);
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// StoredTime
///////////////////////////////////////////////////////////
// MOA checked

StoredTime::StoredTime(uint8_t incRatio, uint8_t decRatio, long minTime, long maxTime)
    : incRatio(incRatio),
      decRatio(decRatio),
      minTime(minTime),
      maxTime(maxTime),
      storedTime(minTime),
      lastCycleTimestamp(millis()) {}

void StoredTime::runCycle(bool sensed) {
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
// MOA TBD check

void RunnerCluster::triggerRunner(uint8_t leafID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime) {
    for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( !runner[i].active ) {
            runner[i].start(leafID, numLeds, runnerSpeed, runnerColor, hueChange, hueChangeInterval, glowTime);
            return;
        }
    }
    DEBUG_PRINTLN("ERROR: failed to trigger new runner");
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
// MOA TBD check

void LedRunner::start(uint8_t leafID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime) {
    active = true;
    leafID = leafID;
    numLeds = numLeds;
    runnerSpeed = runnerSpeed;
    runnerColor = runnerColor;
    glowTime = glowTime;
    hueChange = hueChange;
    hueChangeInterval = hueChangeInterval;
    hueLastChanged = cycleTimestamp;
    startTime = cycleTimestamp;
}

void LedRunner::updateHue() {
    if (hueLastChanged + hueChangeInterval > cycleTimestamp) {
        hueChangeInterval = cycleTimestamp;
        runnerColor.h + hueChange;
    }
}
void LedRunner::updateActiveLed() {
    activeLed = numLeds * runnerSpeed / ( cycleTimestamp - startTime ); 
}
void LedRunner::updateRunner() {
    updateActiveLed();
    updateHue();
    if ( activeLed > numLeds + (glowTime / runnerSpeed) ) {
        active = false;
    }
}
CHSV LedRunner::ledColor(int ledIndex) {
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

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// SenseSensor
///////////////////////////////////////////////////////////

SenseSensor::SenseSensor(const uint8_t sendPin, const uint8_t sensePin): sensePin(sensePin), active(false), disabledSince(0), sensor(sendPin,sensePin) {}
void SenseSensor::doSetup() { 
    sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
}
bool SenseSensor::sense() { 
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
    result.s = back.s + ((frame.s - back.s) / 255) * opacity;
    result.v = back.v + ((255 - back.s) / 255) * opacity;
    return result;
}

///////////////////////////////////////////////////////////

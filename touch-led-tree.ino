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

#define SENSE_STORED_PER_LED 10000
#define SENSE_STORED_PER_LED_100 100
#define DECREASE_SENSE_SPEED 400
#define INCREASE_SENSE_SPEED 200

#define MAX_LEDS_PER_STRIP    60
#define LED_LEAF_MIN_RUNNER_START_INTERVAL_MS 200
#define RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS 4

long cycleTimestamp;

#include "sense_sensor.h"
#include "led_runner.h"
#include "runner_cluster.h"
#include "background.h"
#include "sense_filling.h"
#include "led_leaf.h"
#include "touch_tree.h"

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

///////////////////////////////////////////////////////////
// TouchTree
///////////////////////////////////////////////////////////
    
TouchTree::TouchTree()
    :ledLeaf({
        LedLeaf(1, TOUCH_TREE_SENSE1_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE1_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 0, 4000, 1, 1, 0, CHSV(32,255,255), 1000, 0, &this->runnerCluster),
        LedLeaf(2, TOUCH_TREE_SENSE2_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE2_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 0, 6000, 2, 1, 0, CHSV(64,255,255), 500, 0, &this->runnerCluster),
        LedLeaf(3, TOUCH_TREE_SENSE3_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE3_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 0, 5000, 1, 2, 0, CHSV(96,255,255), 700, 0, &this->runnerCluster),
        LedLeaf(4, TOUCH_TREE_SENSE4_NUM_LEDS, TOUCH_TREE_PIN_TOUCH_SEND, TOUCH_TREE_SENSE4_PIN_RECEIVE, TOUCH_TREE_DEFAULT_ACTIVE_COLOR, TOUCH_TREE_DEFAULT_INACTIVE_COLOR, 0, 10000, 1, 1, 0, CHSV(128,255,255), 300, 0, &this->runnerCluster),
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
// LedLeaf
///////////////////////////////////////////////////////////
    
LedLeaf::LedLeaf(uint8_t stripID,
                 int     numLeds,
                 uint8_t sendPin,
                 uint8_t sensePin,
                 CHSV    backgroundActiveColor,
                 CHSV    backgroundInactiveColor,
                 int     backgroundSenseLedOffset,
                 long    allActiveTimeMs,
                 uint8_t incSenseMsRatio,
                 uint8_t decSenseMsRatio,
                 long    maxSenseOffset,
                 CHSV    runnerColor,
                 long    runnerBaseTime,
                 long    runnerDiffTime,
                 RunnerCluster *runnerCluster)
    : stripID(stripID),
      numLeds(numLeds),
      runnerColor(runnerColor),
      runnerBaseTime(runnerBaseTime),
      runnerDiffTime(runnerDiffTime),
      previousSenseState(false),
      lastRunnerStartTime(0),
      runnerCluster(runnerCluster),
      sensor(sendPin, sensePin),
      senseFilling(incSenseMsRatio, decSenseMsRatio, allActiveTimeMs + maxSenseOffset),
      background(numLeds, backgroundActiveColor, backgroundInactiveColor, backgroundSenseLedOffset) {}

void LedLeaf::runCycle(){
    // Read in the sensor
    bool sensed =  sensor.sense();
    senseFilling.calculateCycle(sensed);
    if ( sensed == true && previousSenseState == false ) {
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
// Background
///////////////////////////////////////////////////////////

Background::Background(int numLeds, CHSV activeColor, CHSV inactiveColor, int ledActiveOffset)
    : numLeds(numLeds), ledActiveOffset(ledActiveOffset), activeColor(activeColor), inactiveColor(inactiveColor) {
    }

CHSV Background::getLedBackColor(int ledIndex, long senseValue) {
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// SenseFilling
///////////////////////////////////////////////////////////

SenseFilling::SenseFilling(uint8_t incSenseMsRatio, uint8_t decSenseMsRatio, long maxSense): incSenseMsRatio(incSenseMsRatio), decSenseMsRatio(decSenseMsRatio), maxSense(maxSense), accumulatedSense(0), lastSenseTimestamp(millis()) {}

void SenseFilling::calculateCycle(bool sensed) {
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
    lastSenseTimestamp = cycleTimestamp;
}

long SenseFilling::getCurrentSense() {
    return accumulatedSense;
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// RunnerCluster
///////////////////////////////////////////////////////////

void RunnerCluster::triggerRunner(uint8_t stripID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime) {
    for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( !runner[i].active ) {
            runner[i].start(stripID, numLeds, runnerSpeed, runnerColor, hueChange, hueChangeInterval, glowTime);
            return;
        }
    }
    DEBUG_PRINTLN("failed to trigger new runner");
}

CHSV RunnerCluster::getLedSprite(uint8_t stripID, int ledIndex) {
    CHSV ledSprite = HSV_COLOR_TRANSPARENT;
    for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( runner[i].active && ( runner[i].stripID == stripID ) ) {
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

void LedRunner::start(uint8_t stripID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime) {
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

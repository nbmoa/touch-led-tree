
//#define DEBUG 1
//#define ENABLE_DEBUG_PRINTS 1
//#define DEBUG_CYCLES 1
//#define DEBUG_SENSOR 1
//#define DEBUG_SENSOR_RAW_SENSE 1
//#define DEBUG_RUNNER 1
//#define DEBUG_RUNNER_GLOW 1
//#define DEBUG_RUNNER_ACTIVE_LED 1
//#define DEBUG_SPRITE 1

#define CONFIG_SENSE_ACTIVE_THREASHOLD        100
#define CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS 10000
#define CONFIG_MAX_LEDS_PER_STRIP 12
#define CONFIG_SENSE1_NUM_LEDS    12
#define CONFIG_SENSE2_NUM_LEDS    12
#define CONFIG_SENSE3_NUM_LEDS    12
#define CONFIG_SENSE4_NUM_LEDS    12

#define CONFIG_BACKGROUND_ACTIVE_V   255
#define CONFIG_BACKGROUND_INACTIVE_V 60
#define CONFIG_TRANSPARENT_V         0

#define CONFIG_MAX_ACTIVE_RUNNERS 16
#define CONFIG_RUNNER_GLOW_NUM_LEDS 4
//#define CONFIG_RUNNER_COLOR       CHSV(64,255,255)
#define CONFIG_RUNNER_HUE_CHANGE  20
#define CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS 200
#define CONFIG_RUNNER_START_HUE_INCREMENT 16

// PIN configuration
#define CONFIG_SENSE1_PIN_SEND    2
#define CONFIG_SENSE1_PIN_RECEIVE 3
#define CONFIG_SENSE1_PIN_LED     4
#define CONFIG_SENSE2_PIN_SEND    5
#define CONFIG_SENSE2_PIN_RECEIVE 6
#define CONFIG_SENSE2_PIN_LED     7
#define CONFIG_SENSE3_PIN_SEND    8
#define CONFIG_SENSE3_PIN_RECEIVE 9
#define CONFIG_SENSE3_PIN_LED     10
#define CONFIG_SENSE4_PIN_SEND    11
#define CONFIG_SENSE4_PIN_RECEIVE 12
#define CONFIG_SENSE4_PIN_LED     13

#define CONFIG_RAINBOW_DURATION_MS   60000
#define CONFIG_RAINBOW_FADE_INTERVAL 2
#define CONFIG_OVERLAY_SPEED         4
#define CONFIG_RAINBOW_H_INTERVAL          1
#define CONFIG_RAINBOW_H_FINISHED_INTERVAL 3
#define CONFIG_RAINBOW_NEEDED_RUNNERS_FOR_RETRIGGER 10

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
        LedLeaf(1, TOUCH_TREE_SENSE1_NUM_LEDS, CONFIG_SENSE1_PIN_SEND, TOUCH_TREE_SENSE1_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 1000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
        LedLeaf(2, TOUCH_TREE_SENSE2_NUM_LEDS, CONFIG_SENSE2_PIN_SEND, TOUCH_TREE_SENSE2_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 1000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
        LedLeaf(3, TOUCH_TREE_SENSE3_NUM_LEDS, CONFIG_SENSE3_PIN_SEND, TOUCH_TREE_SENSE3_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 1000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
        LedLeaf(4, TOUCH_TREE_SENSE4_NUM_LEDS, CONFIG_SENSE4_PIN_SEND, TOUCH_TREE_SENSE4_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 1000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
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

    rainbowH += CONFIG_RAINBOW_H_INTERVAL;

    if (rainbowStartTime == 0 ) {
        if ( rainbowS > 0 ) {
            if ( rainbowS - CONFIG_RAINBOW_FADE_INTERVAL < 0 ) {
                rainbowS = 0;
            } else {
                rainbowS -= CONFIG_RAINBOW_FADE_INTERVAL;
            }
        }
    } else {
        rainbowH += CONFIG_RAINBOW_H_FINISHED_INTERVAL;

        if ( cycleTimestamp < (rainbowStartTime + CONFIG_RAINBOW_DURATION_MS) ) {
            if (rainbowS + CONFIG_RAINBOW_FADE_INTERVAL > 255 ) {
                this->rainbowS = 255;
            } else {
                this->rainbowS += CONFIG_RAINBOW_FADE_INTERVAL;
            }
        } else {
            if ( runnerCluster.executedRunners > CONFIG_RAINBOW_NEEDED_RUNNERS_FOR_RETRIGGER ) {
                PRINTLN("INFO: retrigger rainbow dance");
                triggerRainbow();
            } else {
                rainbowStartTime = 0;
                reset();
            }
        }
    }

    bool done = true;
    for (int index = 0; index < TOUCH_TREE_NUM_STRIPS; index++) {
        ledLeaf[index].runCycle(rainbowH + (index * 64), rainbowS, rainbowStartTime != 0);
        if ( ledLeaf[index].storedTime.storedTime < ledLeaf[index].background.timePerLed * ledLeaf[index].numLeds ) {
            done = false;
        }
    }
    
    if (done) {
        if (rainbowStartTime == 0) {
            PRINTLN("INFO: do the final rainbow dance");
            triggerRainbow();
        }
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

void TouchTree::triggerRainbow() {
    rainbowStartTime = cycleTimestamp;
    runnerCluster.reset();
}
 
void TouchTree::reset() {
    for (int index = 0; index < TOUCH_TREE_NUM_STRIPS; index++) {
        ledLeaf[index].reset();
    }
}
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// LedLeaf
///////////////////////////////////////////////////////////
    
LedLeaf::LedLeaf(uint8_t leafID,
                 int     numLeds,
                 uint8_t sendPin,
                 uint8_t sensePin,
                 uint8_t backgroundActiveColorV,
                 uint8_t backgroundInactiveColorV,
                 long    timePerLed,
                 long    timeMinStored,
                 long    timeMaxStoredOffset,
                 uint8_t timeIncRatio,
                 uint8_t timeDecRatio,
                 long    runnerBaseTime,
                 long    runnerDiffTime,
                 RunnerCluster *runnerCluster)
    : leafID(leafID),
      numLeds(numLeds),
      runnerBaseTime(runnerBaseTime),
      runnerDiffTime(runnerDiffTime),
      previousSenseState(false),
      lastRunnerStartTime(0),
      fullFade(0),
      hueOffset((leafID - 1) * 64),
      runnerCluster(runnerCluster),
      sensor(sendPin, sensePin),
      storedTime(timeIncRatio, timeDecRatio, timeMinStored, (numLeds * timePerLed) + timeMaxStoredOffset),
      background(backgroundActiveColorV, backgroundInactiveColorV, timePerLed) {
}

void LedLeaf::runCycle(uint8_t rainbowH, uint8_t rainbowS, bool finalDance){
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
    if ( !finalDance ) {
        storedTime.update(sensed);
    
        if ( overlayV != 0 ) {
            if ( overlayV - CONFIG_OVERLAY_SPEED < 0 ) {
                this->overlayV = 0;
            } else {
                this->overlayV -= CONFIG_OVERLAY_SPEED;
            }
        }
    } else {
        this->overlayV = 255;
    }
    
    // Handle new timers
    if ( sensed == true && previousSenseState == false ) {
        //try to start a new runner if LED_LEAF_MIN_RUNNER_START_INTERVAL_MS already elapsed since last started runner
        if ( cycleTimestamp - lastRunnerStartTime > LED_LEAF_MIN_RUNNER_START_INTERVAL_MS ) {
            this->hueOffset += CONFIG_RUNNER_START_HUE_INCREMENT;
//            PRINTDECLN("MOA hueOffset: ", hueOffset);
            if ( leafID == 1 || leafID == 3 ) {
                runnerCluster->triggerRunner(leafID, numLeds, runnerBaseTime, rainbowH + hueOffset + 128, random(LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE), LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS, LED_LEAF_DEFAULT_RUNNER_GLOW_NUM_LEDS);                
            } else {
                runnerCluster->triggerRunner(leafID, numLeds, runnerBaseTime + random(runnerDiffTime), rainbowH + hueOffset + 128, random(LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE), LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS, LED_LEAF_DEFAULT_RUNNER_GLOW_NUM_LEDS);                
            }
        }
    }
    previousSenseState = sensed;

    // update the leds based on the stored time and the active runners of the leaf
    for ( int i = 0; i < numLeds; i++ ) {
        CHSV ledBackColor = background.getLedBackColor(i, storedTime.storedTime, rainbowH + hueOffset, rainbowS, overlayV);
        CHSV ledSprite = runnerCluster->getLedSprite(leafID, i);

        CHSV ledValue = draw(ledBackColor, ledSprite);

        leds[i] = ledValue;
    }
}

void LedLeaf::doSetup() {
    // led initialization is done in the TouchTree

    // setup the sensor
    sensor.doSetup();
}

void LedLeaf::reset() {
    storedTime.reset();
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Background
///////////////////////////////////////////////////////////
// MOA OK

Background::Background(uint8_t activeColorV,
                       uint8_t inactiveColorV,
                       long timePerLed)
    : activeColorV(activeColorV),
      inactiveColorV(inactiveColorV),
      timePerLed(timePerLed) {
}

CHSV Background::getLedBackColor(int ledIndex,long storedTime, uint8_t backH, uint8_t backS, uint8_t overlayV) {
    long storedTimeNeededForLed = ledIndex * timePerLed;

    uint8_t backV;
    // its inactive
    if ( storedTime >= storedTimeNeededForLed + timePerLed ) {
        backV = activeColorV;
    } else if ( storedTime < storedTimeNeededForLed ) {
        backV = inactiveColorV;
    } else {
        int activePercent = 100 * ( storedTime - storedTimeNeededForLed ) / timePerLed;
//        if ( activePercent != 0 ) {
//            DEBUG_PRINT("fade-led[");
//            DEBUG_PRINTDEC(ledIndex);
//            DEBUG_PRINTDECLN("]-percent: ", activePercent);
//        }
        backV = fade(inactiveColorV, activeColorV, activePercent);    
    }
    if ( overlayV > backV ) {
        backV = overlayV;
    }
    return CHSV(backH-ledIndex, backS, backV);
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

void StoredTime::reset() {
    this->storedTime = minTime;
    this->lastCycleTimestamp = millis();
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// RunnerCluster
///////////////////////////////////////////////////////////
// MOA checked

void RunnerCluster::triggerRunner(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, long hueChangeInterval, int glowNumLeds) {
    for ( int i = 0; i < RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( !runner[i].active ) {
            runner[i].start(leafID, numLeds, runnerSpeed, runnerColorH, hueChange, hueChangeInterval, glowNumLeds);
            executedRunners += 1;
            return;
        }
    }
    PRINTLN("ERROR: failed to trigger new runner");
}

CHSV RunnerCluster::getLedSprite(uint8_t leafID, int ledIndex) {
    CHSV ledSprite = CHSV(0, 255, CONFIG_TRANSPARENT_V);

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
 void RunnerCluster::reset() {
    this->executedRunners = 0;
 }
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// LedRunner
///////////////////////////////////////////////////////////
// MOA checked

void LedRunner::start(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, long hueChangeInterval, int glowNumLeds) {
    this->active = true;
    this->leafID = leafID;
    this->numLeds = numLeds;
    this->runnerSpeed = runnerSpeed;
    this->runnerLedSpeed = runnerSpeed / numLeds;
    this->runnerColorH = runnerColorH;
    this->glowNumLeds = glowNumLeds;
    this->hueChange = hueChange;
    this->hueChangeInterval = hueChangeInterval;
    this->hueLastChanged = cycleTimestamp;
    this->startTime = cycleTimestamp - runnerLedSpeed;
    updateActiveLed();

#ifdef DEBUG_RUNNER
    DEBUG_PRINT("runner - start, leafID: ");
    DEBUG_PRINTDEC(leafID);
    DEBUG_PRINT(", color: ");
    DEBUG_PRINTDEC(runnerColorH);
    DEBUG_PRINTDECLN(", glowNumLeds: ", glowNumLeds);
#endif

}

void LedRunner::updateHue() {
    if ( cycleTimestamp > hueLastChanged + hueChangeInterval ) {
        this->hueLastChanged = cycleTimestamp;
        this->runnerColorH += hueChange;
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
    if ( ( ledIndex <= activeLed + 1  ) && ( ledIndex > activeLed - glowNumLeds ) ) {
//        if ( ledIndex == activeLed ) {
//            return CHSV(runnerColorH, 255, 255);
//        } else if ( ledIndex == activeLed + 1 ) {
        if ( ledIndex == activeLed + 1 ) {
            uint8_t fadeInH = 255 * ( ( cycleTimestamp - startTime ) % runnerLedSpeed) / runnerLedSpeed;
            return CHSV(runnerColorH, 255, fadeInH);
        } else {
            long glowTime = (glowNumLeds * runnerLedSpeed);
            long glowDiff = cycleTimestamp - startTime - ( ledIndex * runnerLedSpeed);
            uint8_t fadeOutH = 255 * ( glowTime - glowDiff ) / glowTime;

//            if (ledIndex == 5) {
//                DEBUG_PRINT("time: ");
//                DEBUG_PRINTDEC(cycleTimestamp - startTime);
//                DEBUG_PRINT(", glowDiff: ");
//                DEBUG_PRINTDEC(glowDiff);
//                DEBUG_PRINT(", speed: ");
//                DEBUG_PRINTDEC(runnerLedSpeed);
//                DEBUG_PRINT(", glowTime: ");
//                DEBUG_PRINTDEC(glowTime);
//                DEBUG_PRINTDECLN(", fadeOutH: ", fadeOutH);
//            }
            return CHSV(runnerColorH, 255, fadeOutH);
        }
    } else {
        return CHSV(runnerColorH, 255, CONFIG_TRANSPARENT_V);
    }
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// SenseSensor
///////////////////////////////////////////////////////////
// MOA checked

SenseSensor::SenseSensor(const uint8_t sendPin, const uint8_t sensePin): sendPin(sendPin), sensePin(sensePin), active(false), disabledSince(-SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS), sensor(sendPin,sensePin) {
}

void SenseSensor::doSetup() { 
    DEBUG_PRINT("sendPin: ");
    DEBUG_PRINTDEC(sendPin);
    DEBUG_PRINTDECLN(", sensePin: ", sensePin);
    
    sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    sensor.set_CS_Timeout_Millis(SENSE_SENSOR_DISABLE_TIMEOUT_MS + 1);
}

bool SenseSensor::sense() { 
    if (active) {
        long startTime = millis();
        long senseVal = sensor.capacitiveSensor(SENSE_SENSOR_MEASUREMENT_SAMPLES);
        if ( (millis() - startTime ) > SENSE_SENSOR_DISABLE_TIMEOUT_MS) {
            PRINTDECLN("WARN: disable sensor because of timeout on pin ", sensePin);
            this->active = false;
            this->disabledSince = startTime;
        }
#ifdef DEBUG_SENSOR_RAW_SENSE
        DEBUG_PRINT("pin: ");
        DEBUG_PRINTDEC(sensePin);
        DEBUG_PRINTDECLN(", raw sense: ", senseVal);
#endif
        return senseVal >= SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD || senseVal <= -SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD;
    } else {
        if ( cycleTimestamp > disabledSince + SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS ) {
            DEBUG_PRINTDECLN("WARN: try to enable sensor because retry interval elapsed on pin ", sensePin);
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

// fade a h/s/v value by the given percentage
uint8_t fade(uint8_t from, uint8_t to, int percent) {
    CHSV result;
    return from + ( percent * (to - from) / 100);
}

// overlay the sprites of the runners
CHSV overlaySprites(CHSV s1, CHSV s2) {
    CHSV result = s2;
    
    // s1 is transparent so just take s2
    if ( s1.v == 0 ) {
        return result;
    }
    
    // pick the highest v and s value
    if ( s1.v > s2.v ) {
        result.v = s1.v;
    }
    if ( s1.s > s2.s ) {
        result.s = s1.s;
    }

    // get median for the h value
    result.h = s1.h + ((s2.h - s1.h) / 2);

    // yeah we have the result
    return result;
}

// draw the fram of the background
CHSV draw(CHSV back, CHSV frame) {
    CHSV result = back;

    // the resulting h value is the median of of front and back
    result.h = (255 * back.h + (frame.h - back.h) * frame.v ) / 255;

    // fade in the s value baed on the frame.v value
    result.s = back.s + (frame.v * (frame.s - back.s) / 255);

    // get h value of  by adding back and frame and caping it at 255
    if ( back.v + frame.v > 255 ) {
        result.v = 255;
    } else {
        result.v = back.v + frame.v;
    }

    // and done
    return result;
}

///////////////////////////////////////////////////////////

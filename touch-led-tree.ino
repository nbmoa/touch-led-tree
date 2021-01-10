
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup the debug environment
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG 1
//#define DEBUG_RUNNER 1
//#define DEBUG_SENSE_SENSOR 0 // turns on logging for all sensors
//#define DEBUG_SENSE_SENSOR CONFIG_LEAF1_PIN_RECEIVE // turns on logging for sensor of leaf1

// TBC
//#define DEBUG_CYCLES 1
//#define DEBUG_SENSE_SENSOR // turns on logging for all sensors
//#define DEBUG_SPRITE 1

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// configuration of the system
// TBD this should be imported from another file for better branching
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// configration of the led hardware type
#define CONFIG_LED_TYPE WS2812B             // WS2812B LED strips are used
#define CONFIG_COLOR_ORDER GRB              // WS2812B LED strips have GRB color order
#define CONFIG_DEFAULT_BRIGHTNESS 64        // TBD what is the range is 64 max?
#define CONFIG_COLOR_TEMPERATUR 0xFF7029    // see https://forum.arduino.cc/index.php?topic=569832.msg3881649#msg3881649
#define CONFIG_BRIGHTNESS_INTERVAL 500

// configuration of the hardware setup
#define CONFIG_NUM_LEAFS 4  
#define CONFIG_MAX_LEDS_PER_LEAF 12
#define CONFIG_LEAF1_NUM_LEDS    12
#define CONFIG_LEAF2_NUM_LEDS    12
#define CONFIG_LEAF3_NUM_LEDS    12
#define CONFIG_LEAF4_NUM_LEDS    12
// PIN configuration - Leaf 1
#define CONFIG_LEAF1_PIN_SEND    2
#define CONFIG_LEAF1_PIN_RECEIVE 3
#define CONFIG_LEAF1_PIN_LED     4
// PIN configuration - Leaf 2
#define CONFIG_LEAF2_PIN_SEND    5
#define CONFIG_LEAF2_PIN_RECEIVE 6
#define CONFIG_LEAF2_PIN_LED     7
// PIN configuration - Leaf 3
#define CONFIG_LEAF3_PIN_SEND    8
#define CONFIG_LEAF3_PIN_RECEIVE 9
#define CONFIG_LEAF3_PIN_LED     10
// PIN configuration - Leaf 4
#define CONFIG_LEAF4_PIN_SEND    11
#define CONFIG_LEAF4_PIN_RECEIVE 12
#define CONFIG_LEAF4_PIN_LED     13

// configuration of the update cycles
#define CONFIG_UPDATE_INTERVAL 100 // 41.666 == 24 update / sec

// TBC

#define CONFIG_SENSE_MEASUREMENT_SAMPLES      1     // samples done per measurement
#define CONFIG_SENSE_DISABLE_TIMEOUT_MS       20    // timeout for the measurement that disables the sense measurement for CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS
#define CONFIG_SENSE_SENSE_ACTIVE_THREASHOLD  100   // sense threashold to count as active 
#define CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS 10000 // retry interval for disabled sensors

#define CONFIG_BACKGROUND_ACTIVE_V   255
#define CONFIG_BACKGROUND_INACTIVE_V 60
#define CONFIG_TRANSPARENT_V         0

#define CONFIG_MAX_ACTIVE_RUNNERS 16
#define CONFIG_RUNNER_GLOW_NUM_LEDS 4
//#define CONFIG_RUNNER_COLOR       CHSV(64,255,255)
#define CONFIG_RUNNER_HUE_CHANGE  20
#define CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS 200
#define CONFIG_RUNNER_START_HUE_INCREMENT 16
#define CONFIG_RUNNER_AUTO_TRIGGER_MS 0   // 0 disables the autorunner

#define CONFIG_MIN_RUNNER_START_INTERVAL_MS          200


#define CONFIG_TREE_DURATION_MS   60000
#define CONFIG_TREE_FADE_INTERVAL     2
#define CONFIG_OVERLAY_SPEED         4
#define CONFIG_TREE_H_INTERVAL              1
#define CONFIG_TREE_H_FINISHED_INTERVAL  3
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
        LedLeaf(1, CONFIG_LEAF1_NUM_LEDS, CONFIG_LEAF1_PIN_SEND, CONFIG_LEAF1_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 2000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
        LedLeaf(2, CONFIG_LEAF2_NUM_LEDS, CONFIG_LEAF2_PIN_SEND, CONFIG_LEAF2_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 2000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
        LedLeaf(3, CONFIG_LEAF3_NUM_LEDS, CONFIG_LEAF3_PIN_SEND, CONFIG_LEAF3_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 2000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
        LedLeaf(4, CONFIG_LEAF4_NUM_LEDS, CONFIG_LEAF4_PIN_SEND, CONFIG_LEAF4_PIN_RECEIVE, CONFIG_BACKGROUND_ACTIVE_V, CONFIG_BACKGROUND_INACTIVE_V, 2000, 1000, 20000, 4, 1, 1000, 4000, &this->runnerCluster),
    }),
    brightness(CONFIG_DEFAULT_BRIGHTNESS),
    runnerCluster() {
}

void TouchTree::setup() {
    for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        ledLeaf[index].doSetup();
        // setup the led pins that control the led strips, based on the sense led pin
        switch(ledLeaf[index].sensor.sensePin) {
            case CONFIG_LEAF1_PIN_RECEIVE:
                FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF1_PIN_LED, CONFIG_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            case CONFIG_LEAF2_PIN_RECEIVE:
                FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF2_PIN_LED, CONFIG_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            case CONFIG_LEAF3_PIN_RECEIVE:
                FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF3_PIN_LED, CONFIG_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            case CONFIG_LEAF4_PIN_RECEIVE:
                FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF4_PIN_LED, CONFIG_COLOR_ORDER>(ledLeaf[index].leds, ledLeaf[index].numLeds); 
                break;
            default:
                PRINTLN("ERROR: led pin strip unknown touch pin");
        }
    }

    // TBD not sure about this
    FastLED.setTemperature(CONFIG_COLOR_TEMPERATUR);
    FastLED.setBrightness(brightness);
}

void TouchTree::loop() {
    // First update the timestamp
    long lastCycleTimestamp = curCycleTimestamp;
    curCycleTimestamp = millis();

    // Update the runners
    runnerCluster.update(curCycleTimestamp);

    treeH += CONFIG_TREE_H_INTERVAL;

    if (levelStartTime == 0 ) {
        if ( treeS > 0 ) {
            if ( treeS - CONFIG_TREE_FADE_INTERVAL < 0 ) {
                treeS = 0;
            } else {
                treeS -= CONFIG_TREE_FADE_INTERVAL;
            }
        }
    } else {
        treeH += CONFIG_TREE_H_FINISHED_INTERVAL;

        if ( curCycleTimestamp < (levelStartTime + CONFIG_TREE_DURATION_MS) ) {
            if (treeS + CONFIG_TREE_FADE_INTERVAL > 255 ) {
                treeS = 255;
            } else {
                treeS += CONFIG_TREE_FADE_INTERVAL;
            }
        } else {
            if ( runnerCluster.executedRunnerCnt > CONFIG_RAINBOW_NEEDED_RUNNERS_FOR_RETRIGGER ) {
                PRINTLN("INFO: retrigger rainbow dance");
                levelUp();
            } else {
                levelStartTime = 0;
                reset();
            }
        }
    }

    bool done = true;
    for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        ledLeaf[index].runCycle(treeH + (index * 64), treeS, levelStartTime != 0, curCycleTimestamp);
        if ( ledLeaf[index].storedTime.storedTime < ledLeaf[index].background.timePerLed * ledLeaf[index].numLeds ) {
            done = false;
        }
    }
    
    if (done) {
        if (levelStartTime == 0) {
            PRINTLN("INFO: level up the tree");
            levelUp();
        }
    }

    if ( curCycleTimestamp > lastBrightnessUpdate + CONFIG_BRIGHTNESS_INTERVAL ) {
        lastBrightnessUpdate = curCycleTimestamp;
        brightness += random(3)-1;
        if ( brightness < CONFIG_DEFAULT_BRIGHTNESS - 10) {
            CONFIG_DEFAULT_BRIGHTNESS - 10;  
        }
        if ( brightness > CONFIG_DEFAULT_BRIGHTNESS + 10) {
            CONFIG_DEFAULT_BRIGHTNESS + 10;  
        }
        FastLED.setBrightness(brightness);
    }
    FastLED.show();
    
    
    long timeRemain = (lastCycleTimestamp + CONFIG_UPDATE_INTERVAL) - millis();
    if (timeRemain > 0) {
        delay(timeRemain);
    }

#ifdef DEBUG_CYCLES
    PRINTDECLN("cycle-time-diff: ", timeRemain);
#endif
};

void TouchTree::levelUp() {
    levelStartTime = curCycleTimestamp;
    runnerCluster.reset();
}
 
void TouchTree::reset() {
    for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
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
      lastRunnerStartTime(-CONFIG_MIN_RUNNER_START_INTERVAL_MS - 1),
      fullFade(0),
      hueOffset((leafID - 1) * 64),
      runnerCluster(runnerCluster),
      sensor(sendPin, sensePin),
      storedTime(timeIncRatio, timeDecRatio, timeMinStored, (numLeds * timePerLed) + timeMaxStoredOffset),
      background(backgroundActiveColorV, backgroundInactiveColorV, timePerLed) {
}

void LedLeaf::runCycle(uint8_t rainbowH, uint8_t rainbowS, bool finalDance, long now){
    // Read in the sensor
    bool sensed =  sensor.sense();
#ifdef DEBUG_SENSOR
    PRINT("leafID: ");
    PRINTDEC(leafID);
    if ( sensed ) {
      PRINTDECLN(", sense: ON,  time: ", storedTime.storedTime);
    } else {
      PRINTDECLN(", sense: OFF, time: ", storedTime.storedTime);
    }
#endif

    // Update storedTime
    if ( !finalDance ) {
        storedTime.update(sensed, now);
    
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
    if ( sensed == true ) {
        if ( previousSenseState == false ) {
            //try to start a new runner if CONFIG_MIN_RUNNER_START_INTERVAL_MS already elapsed since last started runner
            if ( now - lastRunnerStartTime > CONFIG_MIN_RUNNER_START_INTERVAL_MS ) {
                this->hueOffset += CONFIG_RUNNER_START_HUE_INCREMENT;
                if ( leafID == 1 || leafID == 3 ) {
                    long runnerSpeed = runnerBaseTime;
                    runnerCluster->triggerRunner(leafID, numLeds, runnerSpeed, rainbowH + hueOffset + 128, random(CONFIG_RUNNER_HUE_CHANGE * 2) - CONFIG_RUNNER_HUE_CHANGE, CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS, runnerSpeed / 8, runnerSpeed / 4, runnerSpeed / 2, now);                
                    lastRunnerStartTime = now;
                } else {
                    long runnerSpeed = runnerBaseTime + random(runnerDiffTime);
                    runnerCluster->triggerRunner(leafID, numLeds, runnerSpeed, rainbowH + hueOffset + 128, random(CONFIG_RUNNER_HUE_CHANGE * 2) - CONFIG_RUNNER_HUE_CHANGE, CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS, runnerSpeed / 8, runnerSpeed / 4, runnerSpeed / 2, now);                
                    lastRunnerStartTime = now;
                }
            }
        }
    } else {
        if ( CONFIG_RUNNER_AUTO_TRIGGER_MS != 0 && now - lastRunnerStartTime > CONFIG_RUNNER_AUTO_TRIGGER_MS ) {
            long runnerSpeed = runnerBaseTime + runnerDiffTime;
            runnerCluster->triggerRunner(leafID, numLeds, runnerSpeed, rainbowH + hueOffset + 128, random(CONFIG_RUNNER_HUE_CHANGE * 2) - CONFIG_RUNNER_HUE_CHANGE, CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS, runnerSpeed / 8, runnerSpeed / 4, runnerSpeed / 2, now);                
            lastRunnerStartTime = now;
        }
    }
    previousSenseState = sensed;

    // update the leds based on the stored time and the active runners of the leaf
    for ( int i = 0; i < numLeds; i++ ) {
        CHSV ledBackColor = background.getLedBackColor(i, storedTime.storedTime, rainbowH + hueOffset, rainbowS, overlayV);
        CHSV ledSprite = runnerCluster->getLedSprite(leafID, i, now);
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
//            PRINT("fade-led[");
//            PRINTDEC(ledIndex);
//            PRINTDECLN("]-percent: ", activePercent);
//        }
        backV = fade(inactiveColorV, activeColorV, activePercent);    
    }
    if ( overlayV > backV ) {
        backV = overlayV;
    }
    return CHSV(backH-ledIndex, backS, backV);
}

///////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StoredTime
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// constructor for the stored time
StoredTime::StoredTime(uint8_t incRatio, uint8_t decRatio, long minTime, long maxTime)
    : incRatio(incRatio),
      decRatio(decRatio),
      minTime(minTime),
      maxTime(maxTime),
      storedTime(minTime),
      lastCycleTimestamp(millis()) {}

// update updates the stored time based on the given the and now and the lastCycleTimestamp
void StoredTime::update(bool sensed, long now) {
    if (sensed) {
        if ( storedTime < maxTime ) {
            storedTime = storedTime + ( now - lastCycleTimestamp ) * incRatio;
            if ( storedTime > maxTime ) {
                storedTime = maxTime;
            }
        }
    } else {
        if ( storedTime > minTime ) {
            storedTime = storedTime - ( now - lastCycleTimestamp ) * decRatio;
            if ( storedTime < minTime ) { 
                storedTime = minTime;
            }
        }
    }
    lastCycleTimestamp = now;
}

// reset resets the stored time
void StoredTime::reset() {
    this->storedTime = minTime;
    this->lastCycleTimestamp = millis();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RunnerCluster
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// triggerRunner tries to trigger a new runner
void RunnerCluster::triggerRunner(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, long hueChangeInterval, int fadeIn, int actDur, int fadeOut, long now) {
    for ( int i = 0; i < CONFIG_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( !runner[i].active ) {
            runner[i].start(leafID, numLeds, runnerSpeed, runnerColorH, hueChange, hueChangeInterval, fadeIn, actDur, fadeOut, now);
            executedRunnerCnt += 1;
            return;
        }
    }
    PRINTLN("ERROR: failed to trigger new runner");
}

// getLedSprite gets the sprite for a given ledLeaf, overlaying all runners
CHSV RunnerCluster::getLedSprite(uint8_t leafID, int ledIndex, long now) {
    CHSV ledSprite = CHSV(0, 255, CONFIG_TRANSPARENT_V);

    for ( int i = 0; i < CONFIG_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( runner[i].active && ( runner[i].leafID == leafID ) ) {
            ledSprite = overlaySprites(ledSprite, runner[i].getLedColor(ledIndex, now));
        }
    }
    return ledSprite;
}

// update updates all runners of the runner cluster, needed to be called before getLedSprite and triggerRunner
void RunnerCluster::update(long now) {
    for ( int i = 0; i < CONFIG_MAX_ACTIVE_RUNNERS; i++ ) {
        if ( runner[i].active == true ) {
            runner[i].updateRunner(now);
        }
    }
}

// reset resets the exexutedRunners count
void RunnerCluster::reset() {
   this->executedRunnerCnt = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LedRunner
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// starts the runner, overwriting the current state of the runner
void LedRunner::start(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, int hueChangeInterval, int fadeInSpeed, int actLedDuration, int fadeOutSpeed, long now) {
// Update the runner state variables
    this->active = true;
    this->startTime = now;
    this->leafID = leafID;
    this->numLeds = numLeds;
    this->runnerSpeed = runnerSpeed;
    this->runnerLedSpeed = runnerSpeed / numLeds;
    this->runnerColorH = runnerColorH;
    this->hueChange = hueChange;
    this->hueChangeInterval = hueChangeInterval;
    this->hueLastChanged = now;
    this->fadeInSpeed = fadeInSpeed;
    this->actLedDuration = actLedDuration;
    this->fadeOutSpeed = fadeOutSpeed;

#ifdef DEBUG_RUNNER
    PRINT("runner-start - time: ");
    PRINTDEC(startTime);
    PRINT(", leafID: ");
    PRINTDEC(leafID);
    PRINT(", hue: ");
    PRINTDEC(runnerColorH);
    PRINT(", speed: ");
    PRINTDEC(runnerSpeed);
    PRINT(", fadeIn: ");
    PRINTDEC(fadeInSpeed);
    PRINT(", actDur: ");
    PRINTDEC(actLedDuration);
    PRINTDECLN(", fadeOut: ", fadeOutSpeed);
#endif
}

// updateRunner updates the runners hue value and the active state of the runner
// needs to be called once per cycle, before calling getLedColor
void LedRunner::updateRunner(long now) {
    // Updated runner hue if needed
    if ( now > hueLastChanged + hueChangeInterval ) {
        this->hueLastChanged = now;
        this->runnerColorH += hueChange;
    }

    // Update active state of runner so it will be freed for reuse
    if ( now > startTime + numLeds * runnerLedSpeed  + fadeInSpeed + actLedDuration + fadeOutSpeed ) {
        this->active = false;
#ifdef DEBUG_RUNNER
        PRINTDECLN("runner-end   - time: ", startTime);
#endif
    }
}

// getLedColor returns the color of a led for the given moment
// returns CHSV h value of the runner and
//              s 255
//              v defined by the envelope definiton of the runner (0 transperent, 255 solid)
CHSV LedRunner::getLedColor(int ledIndex, long now) {
    long offStart =  ledIndex * runnerLedSpeed + startTime;
    uint8_t colorV = 0;

    if ( now >= offStart  && now < offStart + fadeInSpeed) {
        colorV = 255 * ( now - offStart ) / fadeInSpeed;
    } else if ( now >= offStart + fadeInSpeed && now < offStart + fadeInSpeed + actLedDuration) {
        colorV = 255;
    } else if ( now >= offStart + fadeInSpeed + actLedDuration && now < offStart + fadeInSpeed + actLedDuration + fadeOutSpeed) {
        colorV = 255 * ( fadeOutSpeed - ( now - ( offStart + fadeInSpeed + actLedDuration ) ) ) / fadeOutSpeed;
    }

    return CHSV(runnerColorH, 255, colorV);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SenseSensor
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// constructor of the sense sensor
SenseSensor::SenseSensor(const uint8_t sendPin, const uint8_t sensePin): sendPin(sendPin), sensePin(sensePin), enabled(false), disabledSince(-CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS), sensor(sendPin,sensePin) {
}

// doSetup setups the sensor
void SenseSensor::doSetup() { 
#ifdef DEBUG_SENSE_SENSOR
    PRINT("sendPin: ");
    PRINTDEC(sendPin);
    PRINTDECLN(", sensePin: ", sensePin);
#endif
    
    sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
    sensor.set_CS_Timeout_Millis(CONFIG_SENSE_DISABLE_TIMEOUT_MS + 1);
}

// sense returns true or false for the sensor
bool SenseSensor::sense() { 
    if (enabled) {
        long startTime = millis(); // this needs to use millis so it actually just measures the time for the measurement of the sense sensor
        long senseVal = sensor.capacitiveSensor(CONFIG_SENSE_MEASUREMENT_SAMPLES);
        if ( (millis() - startTime ) > CONFIG_SENSE_DISABLE_TIMEOUT_MS) {
            PRINTDECLN("WARN: disable sensor because of timeout on pin ", sensePin);
            this->enabled = false;
            this->disabledSince = startTime;
        }
#ifdef DEBUG_SENSE_SENSOR
        if ( DEBUG_SENSE_SENSOR == sensePin || DEBUG_SENSE_SENSOR == 0) {
            PRINT("pin: ");
            PRINTDEC(sensePin);
            PRINTDECLN(", sense: ", senseVal);
        }
#endif
        return senseVal >= CONFIG_SENSE_SENSE_ACTIVE_THREASHOLD || senseVal <= -CONFIG_SENSE_SENSE_ACTIVE_THREASHOLD;
    } else {
        if ( millis() > disabledSince + CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS ) {
#ifdef DEBUG_SENSE_SENSOR
            PRINTDECLN("INFO: try to enable sensor because retry interval elapsed on pin ", sensePin);
#endif
            this->enabled = true;
        }
        return enabled; // for disabled sensors this will create pulses in the CONFIG_SENSE_DISABLE_TIMEOUT_MS interval, should create nice runner effects if the sense is offline
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

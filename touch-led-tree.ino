
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup the debug environment
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define DEBUG 1
//#define DEBUG_RUNNER 1
//#define DEBUG_SENSE_SENSOR 0 // turns on logging for all sensors
//#define DEBUG_SENSE_SENSOR CONFIG_LEAF1_PIN_RECEIVE // turns on logging for sensor of leaf1
//#define DEBUG_CYCLE_DELAYS 1

// TBC
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

typedef enum BackgroundType { 
    BACK_TYPE_FADE_V, 
    BACK_TYPE_FADE_V_REVERSE, 
    BACK_TYPE_FADE_S, 
    BACK_TYPE_FADE_S_REVERSE, 
    BACK_TYPE_NO_FADE
} bg_type_t;


#define LEVEL_0 0
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3
#define LEVEL_4 4
#define LEVEL_5 5

#define CONFIG_BACK_TYPE_FADE_V  0
#define CONFIG_BACK_TYPE_FADE_S  1
#define CONFIG_BACK_TYPE_NO_FADE 2

#define CONFIG_MAX_ACTIVE_RUNNERS 16
#define CONFIG_RUNNER_GLOW_NUM_LEDS 4
//#define CONFIG_RUNNER_COLOR       CHSV(64,255,255)
#define CONFIG_RUNNER_HUE_CHANGE  20
#define CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS 200
#define CONFIG_RUNNER_START_HUE_INCREMENT 16
#define CONFIG_RUNNER_AUTO_TRIGGER_MS 0   // 0 disables the autorunner

#define CONFIG_MIN_RUNNER_START_INTERVAL_MS          200

#define CONFIG_LEVEL_DOWN_IDLE_TIMEOUT 600000   // idle for 10 min levels down

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
        LedLeaf(1, CONFIG_LEAF1_NUM_LEDS, CONFIG_LEAF1_PIN_SEND, CONFIG_LEAF1_PIN_RECEIVE, &this->runnerCluster),
        LedLeaf(2, CONFIG_LEAF2_NUM_LEDS, CONFIG_LEAF2_PIN_SEND, CONFIG_LEAF2_PIN_RECEIVE, &this->runnerCluster),
        LedLeaf(3, CONFIG_LEAF3_NUM_LEDS, CONFIG_LEAF3_PIN_SEND, CONFIG_LEAF3_PIN_RECEIVE, &this->runnerCluster),
        LedLeaf(4, CONFIG_LEAF4_NUM_LEDS, CONFIG_LEAF4_PIN_SEND, CONFIG_LEAF4_PIN_RECEIVE, &this->runnerCluster),
    }),
    treeBrightness(CONFIG_DEFAULT_BRIGHTNESS),
    runnerCluster() {
}

void TouchTree::setup() {
    for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        ledLeaf[index].doSetup();
    }

    // Not tries around much with this, see comment on CONFIG_COLOR_TEMPERATUR
    FastLED.setTemperature(CONFIG_COLOR_TEMPERATUR);

    // And now bring the tree to level 0
    setLevel(LEVEL_0);
}

void TouchTree::loop() {
    // First update the timestamp
    long lastCycleTimestamp = curCycleTimestamp;
    curCycleTimestamp = millis();

    // Update the runners
    runnerCluster.update(curCycleTimestamp);

    // now the the normal level opartions
    treeCycle();

    // check and do the level up/down things
    checkLevelChange();

    // play a bit with the brightness each cycle
    updateBrightness();
    // and now update the leds
    FastLED.show();
    
    // do the cycle
    long timeRemain = (lastCycleTimestamp + CONFIG_UPDATE_INTERVAL) - millis();
    if (timeRemain > 0) {
        delay(timeRemain);
    }

#ifdef DEBUG_CYCLE_DELAYS
    PRINTDECLN("cycle-delay: ", timeRemain);
#endif
};

void TouchTree::treeCycle() {
    switch(treeLevel) {
        case LEVEL_0:
            break;
        case LEVEL_1:
            break;
        case LEVEL_2:
            break;
        case LEVEL_3:
            break;
        case LEVEL_4:
            break;
        case LEVEL_5:
            break;
    }
 
    for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        ledLeaf[index].runCycle(treeColorH, curCycleTimestamp);
    }
}

void TouchTree::checkLevelChange() {
    bool canLevelUp = true;
    bool canLevelDown = true;
    for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        if ( !ledLeaf[index].canLevelUp() ) {
            canLevelUp = false;
        }
        if ( !ledLeaf[index].canLevelDown() ) {
            canLevelDown = false;
        }
    }
    // Handling additional global tree checks here
    switch (treeLevel) {
        case LEVEL_0:
            // We should never go below level 0
            canLevelDown = false;
            break;
        case LEVEL_1:
            break;
        case LEVEL_2:
            break;
        case LEVEL_3:
            break;
        case LEVEL_4:
            break;
        case LEVEL_5:
            // We should never go bejond the last level
            canLevelUp = false;
            break;
    }
    if ( canLevelUp ) {
        setLevel(treeLevel + 1);
    } else if ( canLevelDown ) {
        setLevel(treeLevel - 1);
    }
}


void TouchTree::setLevel(uint8_t level) {
    treeLevel = level;
    levelStartTime = curCycleTimestamp;
    runnerCluster.reset();
    for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        switch (treeLevel) {
            case LEVEL_0:
                // fill up the white, no runner
                ledLeaf[index].setLevel(60000,0, 120000, 8, 1, index * 64, BACK_TYPE_FADE_V, 255, 60, 0, 0, 0, 0);
                break;
            case LEVEL_1:
                // fill up the color , no runner
                ledLeaf[index].setLevel(60000,0, 120000, 8, 1, index * 64, BACK_TYPE_FADE_S, 255, 0, 255, 0, 0, 0);
                break;
            case LEVEL_2:
                // each runner earns score, reverse level, fadeout v
                ledLeaf[index].setLevel(60000,5000, 120000, 0, 1, index * 64, BACK_TYPE_FADE_V_REVERSE, 255, 60, 255, 0, 1000, 0);
                break;
            case LEVEL_3:
                // every time a leaf has the same color as the other leaf it gets a score, runner but no score
                ledLeaf[index].setLevel(60000,5000, 120000, 0, 0, index * 64, BACK_TYPE_FADE_V, 255, 60, 255, 0, 1000, 0);
                break;
            case LEVEL_4:
                // sense to fill it up and the last scores need to be done by the runner
                ledLeaf[index].setLevel(60000,0, 120000, 4, 1, index * 64, BACK_TYPE_FADE_S_REVERSE, 255, 0, 0, 255, 1000, 0);
                break;
            case LEVEL_5:
                // TBD
                // this should have a party mode, and go from white to color rainbow back to white, ...
                ledLeaf[index].setLevel(60000,0, 120000, 0, 1, index * 64, BACK_TYPE_NO_FADE, 255, 0, 255, 0, 1000, 5000);
                break;
        }
    }
}

void TouchTree::updateBrightness() {
    if ( curCycleTimestamp > treeBrightnessLastUpdate + CONFIG_BRIGHTNESS_INTERVAL ) {
        treeBrightnessLastUpdate = curCycleTimestamp;
        treeBrightness += random(3)-1;
        if ( treeBrightness < CONFIG_DEFAULT_BRIGHTNESS - 10) {
            CONFIG_DEFAULT_BRIGHTNESS - 10;  
        }
        if ( treeBrightness > CONFIG_DEFAULT_BRIGHTNESS + 10) {
            CONFIG_DEFAULT_BRIGHTNESS + 10;  
        }
        FastLED.setBrightness(treeBrightness);
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
                 RunnerCluster *runnerCluster)
    : leafID(leafID),
      numLeds(numLeds),
      runnerCluster(runnerCluster),
      sensor(sendPin, sensePin),
      lastRunnerStartTime(-100000) {
}


void LedLeaf::setLevel(
    long scoreNextLevel,
    long scoreMin,
    long scoreMax,
    uint8_t scoreIncRatio,
    uint8_t scoreDecRatio,
    uint8_t colorHueOffset,
    uint8_t backType,
    uint8_t backActiveV,
    uint8_t backInactiveV,
    uint8_t backActiveS,
    uint8_t backInactiveS,
    long runnerBaseTime,
    long runnerDiffTime
    ) {

    this->sensePreviousState = false;
    this->score = scoreMin;
    this->scoreNextLevel = scoreNextLevel;
    this->scorePerLed = scoreNextLevel / numLeds;
    this->scoreMin = scoreMin;
    this->scoreMax = scoreMax;
    this->scoreIncTimeRatio = scoreIncRatio;
    this->scoreDecTimeRatio = scoreDecRatio;
    this->leafColorHueOffset = colorHueOffset;
    this->backCompositionType = backType;
    this->backActiveColorV = backActiveV;
    this->backInactiveColorV = backInactiveV;
    this->backActiveColorS = backActiveS;
    this->backInactiveColorS = backInactiveS;
    this->runnerBaseTime = runnerBaseTime;
    this->runnerDiffTime = runnerDiffTime;
}

void LedLeaf::doSetup() {
    // setup the led pins that control the led strips, based on the sense led pin
    switch(sensor.sensePin) {
        case CONFIG_LEAF1_PIN_RECEIVE:
            FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF1_PIN_LED, CONFIG_COLOR_ORDER>(leds, numLeds); 
            break;
        case CONFIG_LEAF2_PIN_RECEIVE:
            FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF2_PIN_LED, CONFIG_COLOR_ORDER>(leds, numLeds); 
            break;
        case CONFIG_LEAF3_PIN_RECEIVE:
            FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF3_PIN_LED, CONFIG_COLOR_ORDER>(leds, numLeds); 
            break;
        case CONFIG_LEAF4_PIN_RECEIVE:
            FastLED.addLeds<CONFIG_LED_TYPE, CONFIG_LEAF4_PIN_LED, CONFIG_COLOR_ORDER>(leds, numLeds); 
            break;
        default:
            PRINTLN("ERROR: led pin strip unknown touch pin");
        }

    // setup the sensor
    sensor.doSetup();
}

void LedLeaf::runCycle(uint8_t treeH, long now){
    uint8_t leafH = treeH + leafColorHueOffset;
    
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
    
    // Handle new timers
    if ( sensed == true ) {
        if ( sensePreviousState == false ) {
            //try to start a new runner if CONFIG_MIN_RUNNER_START_INTERVAL_MS already elapsed since last started runner
            if ( now - lastRunnerStartTime > CONFIG_MIN_RUNNER_START_INTERVAL_MS ) {
                this->leafColorHueOffset += CONFIG_RUNNER_START_HUE_INCREMENT;
                if ( leafID == 1 || leafID == 3 ) {
                    long runnerSpeed = runnerBaseTime;
                    runnerCluster->triggerRunner(leafID, numLeds, runnerSpeed, leafH + 128, random(CONFIG_RUNNER_HUE_CHANGE * 2) - CONFIG_RUNNER_HUE_CHANGE, CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS, runnerSpeed / 8, runnerSpeed / 4, runnerSpeed / 2, now);                
                    lastRunnerStartTime = now;
                } else {
                    long runnerSpeed = runnerBaseTime + random(runnerDiffTime);
                    runnerCluster->triggerRunner(leafID, numLeds, runnerSpeed, leafH + 128, random(CONFIG_RUNNER_HUE_CHANGE * 2) - CONFIG_RUNNER_HUE_CHANGE, CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS, runnerSpeed / 8, runnerSpeed / 4, runnerSpeed / 2, now);                
                    lastRunnerStartTime = now;
                }
            }
        }
    } else {
        if ( CONFIG_RUNNER_AUTO_TRIGGER_MS != 0 && now - lastRunnerStartTime > CONFIG_RUNNER_AUTO_TRIGGER_MS ) {
            long runnerSpeed = runnerBaseTime + runnerDiffTime;
            runnerCluster->triggerRunner(leafID, numLeds, runnerSpeed, leafH + 128, random(CONFIG_RUNNER_HUE_CHANGE * 2) - CONFIG_RUNNER_HUE_CHANGE, CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS, runnerSpeed / 8, runnerSpeed / 4, runnerSpeed / 2, now);                
            lastRunnerStartTime = now;
        }
    }
    sensePreviousState = sensed;

    // update the leds based on the stored time and the active runners of the leaf
    for ( int i = 0; i < numLeds; i++ ) {
        CHSV ledBackColor = getLedBackColor(i, score, leafH);
        CHSV ledSprite = runnerCluster->getLedSprite(leafID, i, now);
        CHSV ledValue = draw(ledBackColor, ledSprite);

        leds[i] = ledValue;
    }
}

void LedLeaf::reset() {
    score = scoreMin;
}

bool LedLeaf::canLevelUp() {
    if ( score >= scoreNextLevel ) {
        return true;
    }
    return false;
}

bool LedLeaf::canLevelDown() {
    if ( millis() > lastSenseTimestamp + CONFIG_LEVEL_DOWN_IDLE_TIMEOUT ) {
        return true;
    }
    return false;
}

// update updates the score based on the given the and now and the lastCycleTimestamp
void LedLeaf::updateScore(bool sensed, long now) {
    if (sensed) {
        if ( score < scoreMax ) {
            score += ( now - lastCycleTimestamp ) * scoreIncTimeRatio;
            if ( score > scoreMax ) {
                score = scoreMax;
            }
        }
    } else {
        if ( score > scoreMin ) {
            score -= ( now - lastCycleTimestamp ) * scoreDecTimeRatio;
            if ( score < scoreMin ) { 
                score = scoreMin;
            }
        }
    }
    lastCycleTimestamp = now;
}

CHSV LedLeaf::getLedBackColor(int ledIndex,long score, uint8_t backH) {
    long scoreNeededForLed = ledIndex * scorePerLed;

    uint8_t backV = backActiveColorV;
    uint8_t backS = backActiveColorS;

    switch(backCompositionType) {
        case BACK_TYPE_FADE_V:
            // first handle the simple leds
            if ( score >= scoreNeededForLed + scorePerLed ) {
                backV = backActiveColorV;
            } else if ( score < scoreNeededForLed ) {
                backV = backInactiveColorV;
            } else {
                // And now the last that needs fading
                int activePercent = 100 * ( score - scoreNeededForLed ) / scorePerLed;
                backV = fade(backInactiveColorV, backActiveColorV, activePercent);    
            }
            break;;
        case BACK_TYPE_FADE_S:
            // first handle the simple leds
            if ( score >= scoreNeededForLed + scorePerLed ) {
                backS = backActiveColorS;
            } else if ( score < scoreNeededForLed ) {
                backS = backInactiveColorS;
            } else {
                // And now the last that needs fading
                int activePercent = 100 * ( score - scoreNeededForLed ) / scorePerLed;
                backS = fade(backInactiveColorS, backActiveColorS, activePercent);    
            }
            break;;
        case BACK_TYPE_NO_FADE:
            break;;
        default:
            PRINTDECLN("WARN: backCompositionType not handled ", backCompositionType);
    }

    return CHSV(backH, backS, backV);
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

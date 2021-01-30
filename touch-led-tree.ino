
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// debug environment
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// variables to controll the debug output
////////////////////////////////////////////////////////////////////////////
#define DEBUG 1
//#define DEBUG_RUNNER 1
//#define DEBUG_SENSE_SENSOR 0 // turns on logging for all sensors
//#define DEBUG_SENSE_SENSOR CONFIG_LEAF1_PIN_RECEIVE // turns on logging for sensor of leaf1
//#define DEBUG_CYCLE_DELAYS 1

////////////////////////////////////////////////////////////////////////////
// custom hardware configuration
////////////////////////////////////////////////////////////////////////////

// include the hardware config
////////////////////////////////////////////////////////////////////////////
#include "hardware.h"

// possible entries of the hardware config
////////////////////////////////////////////////////////////////////////////
#ifndef CONFIG_NUM_LEAFS
#define CONFIG_NUM_LEAFS         CONFIG_NUM_LEAFS_DEFAULT
#endif

#ifndef CONFIG_MAX_LEDS_PER_LEAF
#define CONFIG_MAX_LEDS_PER_LEAF CONFIG_MAX_LEDS_PER_LEAF_DEFAULT
#endif

#ifndef CONFIG_LEVEL_TYPE
#define CONFIG_LEVEL_TYPE CONFIG_LEVEL_TYPE_DEFAULT
#endif

#ifndef CONFIG_SENSE_TYPE
#define CONFIG_SENSE_TYPE CONFIG_SENSE_TYPE_100M_RESISTOR
#endif

#ifndef CONFIG_INITIAL_LEVEL
#define CONFIG_INITIAL_LEVEL CONFIG_INITIAL_LEVEL_DEFAULT
#endif
////////////////////////////////////////////////////////////////////////////
// configuration of the hardware setup
////////////////////////////////////////////////////////////////////////////

// config that can be overwritten in the hardware.h file
////////////////////////////////////////////////////////////////////////////
#define CONFIG_NUM_LEAFS_DEFAULT         4
#define CONFIG_MAX_LEDS_PER_LEAF_DEFAULT 12

// hardware config, that cannot be overwritten
////////////////////////////////////////////////////////////////////////////
#define CONFIG_LEAF1_NUM_LEDS    CONFIG_MAX_LEDS_PER_LEAF
#define CONFIG_LEAF2_NUM_LEDS    CONFIG_MAX_LEDS_PER_LEAF
#define CONFIG_LEAF3_NUM_LEDS    CONFIG_MAX_LEDS_PER_LEAF
#define CONFIG_LEAF4_NUM_LEDS    CONFIG_MAX_LEDS_PER_LEAF

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

// configration of the led hardware type
#define CONFIG_LED_TYPE             WS2812B             // WS2812B LED strips are used
#define CONFIG_COLOR_ORDER          GRB              // WS2812B LED strips have GRB color order

////////////////////////////////////////////////////////////////////////////
// app configuration
////////////////////////////////////////////////////////////////////////////

// update intervals
////////////////////////////////////////////////////////////////////////////
#define CONFIG_UPDATE_INTERVAL      100 // 41.666 == 24 update / sec
#define CONFIG_BRIGHTNESS_INTERVAL  500

// color config
////////////////////////////////////////////////////////////////////////////
#define CONFIG_DEFAULT_BRIGHTNESS     64        // TBD what is the range is 64 max?
#define CONFIG_COLOR_TEMPERATUR       0xFF7029    // see https://forum.arduino.cc/index.php?topic=569832.msg3881649#msg3881649
//#define CONFIG_BACKGROUND_ACTIVE_V    255
//#define CONFIG_BACKGROUND_INACTIVE_V  60
#define CONFIG_TRANSPARENT_V          0
#define CONFIG_SCORE_FLASH_MS         200

// sense conifg
////////////////////////////////////////////////////////////////////////////
#define CONFIG_SENSE_TIMEOUT_MS               100    // timeout for the measurement that disables the sense measurement for CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS
#define CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS 5000 // retry interval for disabled sensors

#if  CONFIG_SENSE_TYPE == CONFIG_SENSE_TYPE_100K_RESISTOR
 #define CONFIG_SENSE_MEASUREMENT_SAMPLES      10     // samples done per measurement
 #define CONFIG_SENSE_SENSE_ACTIVE_THREASHOLD  20   // sense threashold to count as active 
#else
#ifdef CONFIG_SENSE_TYPE == CONFIG_SENSE_TYPE_100M_RESISTOR
 #define CONFIG_SENSE_MEASUREMENT_SAMPLES      1     // samples done per measurement
 #define CONFIG_SENSE_SENSE_ACTIVE_THREASHOLD  100   // sense threashold to count as active 
#endif
#endif

// background config
////////////////////////////////////////////////////////////////////////////
typedef enum BackgroundType {
  BACK_TYPE_FADE_V,
  BACK_TYPE_FADE_V_REVERSE,
  BACK_TYPE_FADE_S,
  BACK_TYPE_FADE_S_REVERSE,
  BACK_TYPE_NO_FADE
} bg_type_t;

// level config
////////////////////////////////////////////////////////////////////////////
#define CONFIG_LEVEL_TYPE_DEFAULT      LEVEL_TYPE_PROTO_GAME
#define CONFIG_INITIAL_LEVEL_DEFAULT   LEVEL_0
#define CONFIG_LEVEL_DOWN_IDLE_TIMEOUT 600000   // idle for 10 min levels down
#define LEVEL_0 0
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3
#define LEVEL_4 4
#define LEVEL_5 5
typedef enum LevelTypes {
  LEVEL_TYPE_PROTO_GAME,
  LEVEL_TYPE_LAMP
} level_t;

// runner config
#define CONFIG_MIN_RUNNER_START_INTERVAL_MS   200
#define CONFIG_RUNNER_HUE_CHANGE              20
#define CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS  200
#define CONFIG_RUNNER_START_HUE_INCREMENT     16
#define CONFIG_RUNNER_AUTO_TRIGGER_MS         0   // 0 disables the autorunner
#ifdef DEBUG
 #define CONFIG_MAX_ACTIVE_RUNNERS 4
#else
 #define CONFIG_MAX_ACTIVE_RUNNERS 8
#endif

////////////////////////////////////////////////////////////////////////////
// config inculdes
////////////////////////////////////////////////////////////////////////////

// libray includes
////////////////////////////////////////////////////////////////////////////
#include <CapacitiveSensor.h>
// This suppress the pragma warning of FastLED (see https://github.com/FastLED/FastLED/issues/797)
#define FASTLED_INTERNAL
#include "FastLED.h"

// local includes
////////////////////////////////////////////////////////////////////////////
#include "debug.h"
#include "sense_sensor.h"
#include "led_runner.h"
#include "runner_cluster.h"
#include "led_leaf.h"
#include "touch_tree.h"

///////////////////////////////////////////////////////////
// led tree application
///////////////////////////////////////////////////////////

// main
///////////////////////////////////////////////////////////
// defined globaly so it will be a static struct
TouchTree touchTree;

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif
  PRINTLN("INFO: startup tree");
  touchTree.setup();
}

void loop() {
  touchTree.loop();
}

// TouchTree
///////////////////////////////////////////////////////////

TouchTree::TouchTree()
  : ledLeaf( {
  LedLeaf(1, CONFIG_LEAF1_NUM_LEDS, CONFIG_LEAF1_PIN_SEND, CONFIG_LEAF1_PIN_RECEIVE, &this->runnerCluster),
#if CONFIG_NUM_LEAFS >= 2
          LedLeaf(2, CONFIG_LEAF2_NUM_LEDS, CONFIG_LEAF2_PIN_SEND, CONFIG_LEAF2_PIN_RECEIVE, &this->runnerCluster),
#endif
#if CONFIG_NUM_LEAFS >= 3
          LedLeaf(3, CONFIG_LEAF3_NUM_LEDS, CONFIG_LEAF3_PIN_SEND, CONFIG_LEAF3_PIN_RECEIVE, &this->runnerCluster),
#endif
#if CONFIG_NUM_LEAFS >= 4
          LedLeaf(4, CONFIG_LEAF4_NUM_LEDS, CONFIG_LEAF4_PIN_SEND, CONFIG_LEAF4_PIN_RECEIVE, &this->runnerCluster),
#endif
  }),
  runnerCluster(),
  treeType(CONFIG_LEVEL_TYPE),
  treeBrightness(CONFIG_DEFAULT_BRIGHTNESS) {
}

void TouchTree::setup() {
  for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
    ledLeaf[index].doSetup();
  }

  // Not tries around much with this, see comment on CONFIG_COLOR_TEMPERATUR
  FastLED.setTemperature(CONFIG_COLOR_TEMPERATUR);

  // And now bring the tree to level 0
  setLevel(CONFIG_INITIAL_LEVEL);
}

void TouchTree::loop() {
  // First update the timestamp
  long lastCycleTimestamp = curCycleTimestamp;
  curCycleTimestamp = millis();

  // play a bit with the brightness each cycle
  updateBrightness();

  // Update the runners
  runnerCluster.update(curCycleTimestamp);

  // now the the normal level opartions
  runLevel();

  // check and do the level up/down things
  checkLevelChange();

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


void TouchTree::runLevel() {
  switch (treeLevel) {
    case LEVEL_0:
      break;
    case LEVEL_1:
      break;
    case LEVEL_2:
      break;
    case LEVEL_3:
      if (treeType == LEVEL_TYPE_PROTO_GAME) {
      // this needs to be before leaf.runLevel so it outruns the normal sense if colors are equal
          scoreLeafColor();
      }
      if (treeType == LEVEL_TYPE_LAMP) {
      // this needs to be before leaf.runLevel so it outruns the normal sense if colors are equal
         treeColorH += 1;
      }
      break;
    case LEVEL_4:
      break;
    case LEVEL_5:
      break;
  }

  for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
    ledLeaf[index].runLevel(treeColorH, curCycleTimestamp);
  }
}

void TouchTree::scoreLeafColor(){
  for (int i = 0; i < CONFIG_NUM_LEAFS; i++) {
    for (int j = 0; j < CONFIG_NUM_LEAFS; j++) {
      if ( i != j && ledLeaf[i].leafColorHOffset == ledLeaf[j].leafColorHOffset ) {
        ledLeaf[i].scoreSense(true, ledLeaf[i].scoreMax, curCycleTimestamp);
        break;
      }
    }
  }  
}

void TouchTree::checkLevelChange() {
  bool canLevelUp = true;
  bool canLevelDown = true;
  for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
    if ( !ledLeaf[index].canLevelUp() ) {
      canLevelUp = false;
    }
    if (treeType == LEVEL_TYPE_LAMP || !ledLeaf[index].canLevelDown() ) {
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
      if (treeType == LEVEL_TYPE_PROTO_GAME) {
        // We should never go bejond the last level
        canLevelUp = false;
      }
      break;
  }
  if ( canLevelUp ) {
    if (treeType == LEVEL_TYPE_LAMP && treeLevel == LEVEL_3) {
      setLevel(0);
    } else {
      setLevel(treeLevel + 1);
    }
  } else if ( canLevelDown ) {
    setLevel(treeLevel - 1);
  }
}


void TouchTree::setLevel(uint8_t level) {  
  PRINTDECLN("treeLevel:", level);
  treeLevel = level;
  levelStartTime = curCycleTimestamp;
  runnerCluster.reset();
  switch (treeType) {
    case LEVEL_TYPE_PROTO_GAME:
      for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        switch (treeLevel) {
          case LEVEL_0:
            // fill up the white, no runner
            ledLeaf[index].setLevel(LEVEL_0, treeType, 60000, 0, 120000, 8, 1, 120000, 0, index * 64, 0, BACK_TYPE_FADE_V, 255, 60, 0, 0, 0, 0, curCycleTimestamp);
            break;
          case LEVEL_1:
            // fill up the color , no runner
            ledLeaf[index].setLevel(LEVEL_1, treeType, 60000, 0, 120000, 8, 1, 120000, 0, index * 64, 0, BACK_TYPE_FADE_S, 255, 0, 255, 0, 0, 0, curCycleTimestamp);
            break;
          case LEVEL_2:
            // each runner earns score, reverse level, fadeout v
            ledLeaf[index].setLevel(LEVEL_2, treeType, 60000, 5000, 120000, 0, 1, 120000, 4000, index * 64, 0, BACK_TYPE_FADE_V_REVERSE, 255, 60, 255, 0, 1000, 0, curCycleTimestamp);
            break;
          case LEVEL_3:
            // every time a leaf has the same color as the other leaf it gets a score, runner but no score
            ledLeaf[index].setLevel(LEVEL_3, treeType, 60000, 5000, 120000, 2, 1, 0, 0, index * 64, 8, BACK_TYPE_FADE_V, 255, 60, 255, 0, 1000, 0, curCycleTimestamp);
            break;
          case LEVEL_4:
            // sense to fill it up and the last scores need to be done by the runner
            ledLeaf[index].setLevel(LEVEL_4, treeType, 60000, 0, 120000, 4, 1, 50000, 4000, index * 64, 8, BACK_TYPE_FADE_S_REVERSE, 255, 0, 255, 0, 1000, 0, curCycleTimestamp);
            break;
          case LEVEL_5:
            // TBD
            // this should have a party mode, and go from white to color rainbow back to white, ...
            ledLeaf[index].setLevel(LEVEL_5, treeType, 60000, 0, 120000, 1, 1, 120000, 4000, index * 64, 8, BACK_TYPE_NO_FADE, 255, 0, 255, 0, 1000, 5000, curCycleTimestamp);
            break;
        }
      }
      break;
    case LEVEL_TYPE_LAMP:
      for (int index = 0; index < CONFIG_NUM_LEAFS; index++) {
        switch (treeLevel) {
          case LEVEL_0:
            // dimmed lamp
            ledLeaf[index].setLevel(LEVEL_0, treeType, 2000, 0, 120000, 1, 1, 120000, 0, index * 64, 0, BACK_TYPE_NO_FADE, 60, 0, 0, 0, 0, 0, curCycleTimestamp);
            break;
          case LEVEL_1:
            // dimmed lamp
            ledLeaf[index].setLevel(LEVEL_1, treeType, 2000, 0, 120000, 1, 1, 120000, 0, index * 64, 0, BACK_TYPE_NO_FADE, 120, 0, 0, 0, 0, 0, curCycleTimestamp);
            break;
          case LEVEL_2:
            // bright lamp
            ledLeaf[index].setLevel(LEVEL_2, treeType, 2000, 0, 120000, 1, 1, 120000, 0, index * 64, 0, BACK_TYPE_NO_FADE, 255, 0, 0, 0, 0, 0, curCycleTimestamp);
            break;
          case LEVEL_3:
            // party lamp
            ledLeaf[index].setLevel(LEVEL_3, treeType, 10000, 0, 120000, 1, 10, 120000, 4000, index * 64, 8, BACK_TYPE_NO_FADE, 255, 0, 255, 0, 1000, 0, curCycleTimestamp);
            break;
        }
      }
      break;
  }
}

void TouchTree::updateBrightness() {
  if ( curCycleTimestamp > treeBrightnessLastUpdate + CONFIG_BRIGHTNESS_INTERVAL ) {
    treeBrightnessLastUpdate = curCycleTimestamp;
    treeBrightness += random(3) - 1;
    if ( treeBrightness < CONFIG_DEFAULT_BRIGHTNESS - 4) {
      treeBrightness = CONFIG_DEFAULT_BRIGHTNESS - 4;
    }
    if ( treeBrightness > CONFIG_DEFAULT_BRIGHTNESS + 4) {
      treeBrightness = CONFIG_DEFAULT_BRIGHTNESS + 4;
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
    sensor(sendPin, sensePin),
    runnerLastStartTime(-100000),
    runnerCluster(runnerCluster) {
    }


void LedLeaf::setLevel(
  uint8_t level,
  level_t treeType,
  long scoreNextLevel,
  long scoreMin,
  long scoreMax,
  uint8_t scoreIncRatio,
  uint8_t scoreDecRatio,
  long    scoreLevelSenseCap,
  long    scorePerRunner,
  uint8_t colorHOffset,
  uint8_t runnerColorHOffsetChange,
  uint8_t backType,
  uint8_t backActiveV,
  uint8_t backInactiveV,
  uint8_t backActiveS,
  uint8_t backInactiveS,
  long runnerBaseTime,
  long runnerDiffTime,
  long now
) {
  this->level = level;
  this->treeType = treeType;
  this->lastSenseState = false;
  this->score = scoreMin;
  this->scoreNextLevel = scoreNextLevel;
  this->scorePerLed = scoreNextLevel / numLeds;
  this->scoreMin = scoreMin;
  this->scoreMax = scoreMax;
  this->scoreIncTimeRatio = scoreIncRatio;
  this->scoreDecTimeRatio = scoreDecRatio;
  this->scoreLevelSenseCap = scoreLevelSenseCap;
  this->scorePerRunner = scorePerRunner;
  this->leafColorHOffset = colorHOffset;
  this->runnerColorHOffsetChange = runnerColorHOffsetChange;
  this->backCompositionType = backType;
  this->backActiveColorV = backActiveV;
  this->backInactiveColorV = backInactiveV;
  this->backActiveColorS = backActiveS;
  this->backInactiveColorS = backInactiveS;
  this->runnerBaseTime = runnerBaseTime;
  this->runnerDiffTime = runnerDiffTime;
  this->lastActiveTimestamp = now;
  PRINTDEC("lvl:", level);
  PRINTDEC(", scr:", score);
  PRINTDEC(", up:", scoreNextLevel);
  PRINTDEC(", min:", scoreMin);
  PRINTDEC(", max:", scoreMax);
  PRINTDEC(", inc:", scoreIncTimeRatio);
  PRINTDEC(", dec:", scoreDecTimeRatio);
  PRINTDEC(", cap:", scoreLevelSenseCap);
  PRINTDECLN(", run:", scorePerRunner);
}

void LedLeaf::doSetup() {
  // setup the led pins that control the led strips, based on the sense led pin
  switch (sensor.sensePin) {
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

void LedLeaf::runLevel(uint8_t treeH, long now) {
  uint8_t leafH = treeH + leafColorHOffset;

  // Read in the sensor
  bool sensed =  doSense();

  // update hte lastActiveTimestamp used for level down
  if (sensed == true) {
    lastActiveTimestamp = now;
  }

  // eval the sense for the score
  scoreSense(sensed, scoreLevelSenseCap, now);

  // start runner if neccesary
  doRunner(sensed, leafH, now);

  switch (level) {
    case LEVEL_0:
      break;
    case LEVEL_1:
      break;
    case LEVEL_2:
      break;
    case LEVEL_3:
      PRINTDEC("MOA-n:", now);
      PRINTDECLN("lp:", runnerLastStartTime + 2000);
      if (treeType == LEVEL_TYPE_LAMP) {
        if (now > runnerLastStartTime + 2000) {
          startRunner(leafH, now);
        }
      }
      break;
    case LEVEL_4:
      break;
    case LEVEL_5:
      break;
  }

  // update the leds based on the stored time and the active runners of the leaf
  updateLeds(leafH, now);

  // now update the things needed from this cycle for the next cycle
  lastSenseState = sensed;
}

bool LedLeaf::doSense() {
  bool sensed = sensor.sense();
#ifdef DEBUG_SENSOR
  PRINT("leafID: ");
  PRINTDEC(leafID);
  if ( sensed ) {
    PRINTDECLN(", sense: ON,  score: ", score);
  } else {
    PRINTDECLN(", sense: OFF, score: ", score);
  }
#endif
  return sensed;
}

void LedLeaf::doScore(long scoreDiff, long now) {
  if (scoreDiff > 0) {
    doFlash(now);
  }
  score += scoreDiff;
  if (score > scoreMax) {
    score = scoreMax;
  }
  if (score < scoreMin) {
    score = scoreMin;
  }
}

void LedLeaf::doFlash(long now) {
//  flashLeafTimestamp = now;
}
    
// count sense for the score based on the given sense state, now and the lastCycleTimestamp
void LedLeaf::scoreSense(bool sensed, long scoreCap, long now) {
  if (sensed) {
    if ( score < scoreCap ) {
      doScore(( now - lastCycleTimestamp ) * scoreIncTimeRatio, now);
    }
  } else {
    doScore(-( now - lastCycleTimestamp ) * scoreDecTimeRatio, now);
  }
  lastCycleTimestamp = now;
}

void LedLeaf::doRunner(bool sensed, uint8_t leafH, long now) {
  // Handle new timers
  if ( sensed == true ) {
    if ( lastSenseState == false ) {
      //try to start a new runner if CONFIG_MIN_RUNNER_START_INTERVAL_MS already elapsed since last started runner
      if ( now - runnerLastStartTime > CONFIG_MIN_RUNNER_START_INTERVAL_MS ) {
         startRunner(leafH, now);
      }
    }
  }
}

void LedLeaf::startRunner(uint8_t leafH, long now) {
  // update the leaf color offset with the runner offset change value
  this->leafColorHOffset += runnerColorHOffsetChange;

  // get speed of the runner
  if (runnerBaseTime != 0) {
    long runnerSpeed = runnerBaseTime + random(runnerDiffTime);
    uint8_t hChange = random(CONFIG_RUNNER_HUE_CHANGE * 2) - CONFIG_RUNNER_HUE_CHANGE;
    uint8_t runnerH = leafH + 128;
    runnerCluster->triggerRunner(leafID, numLeds, runnerSpeed, runnerH, hChange, CONFIG_RUNNER_HUE_CHANGE_INTERVAL_MS, runnerSpeed / 8, runnerSpeed / 4, runnerSpeed / 2, now);
    doScore(scorePerRunner, now);
    runnerLastStartTime = now;
  }
}

void LedLeaf::updateLeds(uint8_t leafH, long now) {
  for ( int i = 0; i < numLeds; i++ ) {
    CHSV ledBackColor = getLedBackColor(i, score, leafH);
    CHSV ledSprite = runnerCluster->getLedSprite(leafID, i, now);
    CHSV ledValue = draw(ledBackColor, ledSprite);

    if (now < flashLeafTimestamp + CONFIG_SCORE_FLASH_MS) {
      ledValue.v = 255;
    }

    leds[i] = ledValue;
  }
}

bool LedLeaf::canLevelUp() {
  if ( score >= scoreNextLevel ) {
    return true;
  }
  return false;
}

bool LedLeaf::canLevelDown() {
  if ( millis() > (unsigned long)(lastActiveTimestamp + CONFIG_LEVEL_DOWN_IDLE_TIMEOUT) ) {
    return true;
  }
  return false;
}

CHSV LedLeaf::getLedBackColor(int ledIndex, long score, uint8_t backH) {
  int correctedLedIndex = ledIndex;
  switch (backCompositionType) {
    case BACK_TYPE_FADE_V_REVERSE:
    case BACK_TYPE_FADE_S_REVERSE:
      correctedLedIndex = numLeds - ledIndex;
      break;
  }
  long scoreNeededForLed = correctedLedIndex * scorePerLed;

  uint8_t backV = backActiveColorV;
  uint8_t backS = backActiveColorS;

  switch (backCompositionType) {
    case BACK_TYPE_FADE_V:
      if ( score >= scoreNeededForLed + scorePerLed ) {
        // when score reached
        backV = backActiveColorV;
      } else if ( score < scoreNeededForLed ) {
        // when score not reached
        backV = backInactiveColorV;
      } else {
        // while score is reaching
        int activePercent = 100 * ( score - scoreNeededForLed ) / scorePerLed;
        backV = fade(backInactiveColorV, backActiveColorV, activePercent);
      }
      break;;
    case BACK_TYPE_FADE_S:
      if ( score >= scoreNeededForLed + scorePerLed ) {
        // when score reached
        backS = backActiveColorS;
      } else if ( score < scoreNeededForLed ) {
        // when score not reached
        backS = backInactiveColorS;
      } else {
        // while score is reaching
        int activePercent = 100 * ( score - scoreNeededForLed ) / scorePerLed;
        backS = fade(backInactiveColorS, backActiveColorS, activePercent);
      }
      break;;
    case BACK_TYPE_FADE_V_REVERSE:
      if ( score >= scoreNeededForLed + scorePerLed ) {
        // when score reached
        backV = backInactiveColorV;
      } else if ( score < scoreNeededForLed ) {
        // when score not reached
        backV = backActiveColorV;
      } else {
        // while score is reaching
        int activePercent = 100 * ( score - scoreNeededForLed ) / scorePerLed;
        backV = fade(backActiveColorV, backInactiveColorV, activePercent);
      }
      break;;
    case BACK_TYPE_FADE_S_REVERSE:
      if ( score >= scoreNeededForLed + scorePerLed ) {
        // when score reached
        backS = backInactiveColorS;
      } else if ( score < scoreNeededForLed ) {
        // when score not reached
        backS = backActiveColorS;
      } else {
        // while score is reaching
        int activePercent = 100 * ( score - scoreNeededForLed ) / scorePerLed;
        backS = fade(backActiveColorS, backInactiveColorS, activePercent);
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
  PRINTDEC("runner-start - time: ", startTime);
  PRINTDEC(", leafID: ", leafID);
  PRINTDEC(", hue: ", runnerColorH);
  PRINTDEC(", speed: ", runnerSpeed);
  PRINTDEC(", fadeIn: ", fadeInSpeed);
  PRINTDEC(", actDur: ", actLedDuration);
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
SenseSensor::SenseSensor(const uint8_t sendPin, const uint8_t sensePin): sendPin(sendPin), sensePin(sensePin), enabled(false), disabledSince(-CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS), sensor(sendPin, sensePin) {
}

// doSetup setups the sensor
void SenseSensor::doSetup() {
#ifdef DEBUG_SENSE_SENSOR
  PRINTDEC("sendPin: ", sendPin);
  PRINTDECLN(", sensePin: ", sensePin);
#endif

  sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
  sensor.set_CS_Timeout_Millis(CONFIG_SENSE_TIMEOUT_MS + 10);
}

// sense returns true or false for the sensor
bool SenseSensor::sense() {
  if (enabled) {
    long startTime = millis(); // this needs to use millis so it actually just measures the time for the measurement of the sense sensor
    long senseVal = sensor.capacitiveSensor(CONFIG_SENSE_MEASUREMENT_SAMPLES);
    if ( (millis() - startTime ) > CONFIG_SENSE_TIMEOUT_MS) {
      PRINTDECLN("WARN: disable sensor because of timeout on pin ", sensePin);
      this->enabled = false;
      this->disabledSince = startTime;
    }
#ifdef DEBUG_SENSE_SENSOR
    if ( DEBUG_SENSE_SENSOR == sensePin || DEBUG_SENSE_SENSOR == 0) {
      static int i = 0;
      i++;
      if ( i == 1 ) {
        PRINTDEC("pin: ", sensePin);
        PRINTDEC(", sense: ", senseVal);
      } else if ( i == 10 ) {
        PRINTDECLN(" : ", senseVal);
        i = 0;
      }else {
        PRINTDEC(" : ", senseVal);
      }
    }
#endif // DEBUG_SENSE_SENSOR
    return senseVal >= CONFIG_SENSE_SENSE_ACTIVE_THREASHOLD || senseVal <= -CONFIG_SENSE_SENSE_ACTIVE_THREASHOLD;
  } else {
    if ( millis() > (unsigned long)(disabledSince + CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS) ) {
#ifdef DEBUG_SENSE_SENSOR
      PRINTDECLN("INFO: try to enable sensor because retry interval elapsed on pin ", sensePin);
#endif
      this->enabled = true;
    }
    return enabled; // for disabled sensors this will create pulses in the CONFIG_SENSE_TIMEOUT_MS interval, should create nice runner effects if the sense is offline
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

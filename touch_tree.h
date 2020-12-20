#define TOUCH_TREE_NUM_STRIPS 4

#define TOUCH_TREE_LED_TYPE WS2812B             // WS2812B LED strips are used
#define TOUCH_TREE_COLOR_ORDER GRB              // WS2812B LED strips have GRB color order
#define TOUCH_TREE_DEFAULT_BRIGHTNESS 64        // TBD what is the range is 64 max?

#define TOUCH_TREE_CYCLE_INTERVAL 40 // 41.666 == 24 update / sec

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

struct TouchTree {
    long lastCycleTimestamp;

    RunnerCluster      runnerCluster;
    LedLeaf ledLeaf[TOUCH_TREE_NUM_STRIPS];
    
    TouchTree();
    void setup();
    void loop(); 
};

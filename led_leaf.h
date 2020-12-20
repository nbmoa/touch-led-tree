#define LED_LEAF_MAX_LEDS_PER_STRIP    60
#define LED_LEAF_MIN_RUNNER_START_INTERVAL_MS 200
#define LED_LEAF_DEFAULT_RUNNER_SPEED_MS 10000 
#define LED_LEAF_DEFAULT_RUNNER_COLOR CHSV(32,255,255)
#define LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE 1
#define LED_LEAF_DEFAULT_RUNNER_HUE_CHANGE_INTERVAL_MS 40
#define LED_LEAF_DEFAULT_RUNNER_GLOWTIME_MS 3000
struct LedLeaf {
    uint8_t stripID;
    int numLeds;         // Number of leds of the strip

    CHSV runnerColor;    // LED color of the runner
    long runnerBaseTime;
    long runnerDiffTime;
    bool previousSenseState;
    
    long lastRunnerStartTime;

    CRGB leds[LED_LEAF_MAX_LEDS_PER_STRIP];

    RunnerCluster   *runnerCluster;
    SenseSensor     sensor;
    SenseFilling    senseFilling;
    Background      background;

    LedLeaf(uint8_t stripID,
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
            RunnerCluster *runnerCluster);
    void runCycle();
    void doSetup();
};

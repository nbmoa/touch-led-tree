
struct LedLeaf {
    uint8_t leafID;    // ID of the leaf
    int     numLeds;    // Number of leds of the leaf

    long runnerBaseTime;
    long runnerDiffTime;
    bool previousSenseState;
    
    long lastRunnerStartTime;
    uint8_t fullFade;
    uint8_t overlayV;

    uint8_t hueOffset;

    CRGB leds[CONFIG_MAX_LEDS_PER_LEAF];

    RunnerCluster   *runnerCluster;
    SenseSensor     sensor;
    StoredTime      storedTime;
    Background      background;

    LedLeaf(uint8_t leafID,
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
            RunnerCluster *runnerCluster);
    void runCycle(uint8_t rainbowC, uint8_t rainbowS, bool finalDance, long now);
    void doSetup();
    void reset();
};

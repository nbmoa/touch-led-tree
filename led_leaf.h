
struct LedLeaf {
    uint8_t leafID;                 // ID of the leaf
    int     numLeds;                // number of leds of the leaf
//    uint8_t level;                  // current level of the leaf
    long    lastCycleTimestamp;     // the timestamp of the last cycle

    // sense
    SenseSensor sensor;             // touch sensor of the leaf
    bool        sensePreviousState; // previos sense state of the leaf
    long        lastSenseTimestamp; // the timestamp of the last sense
    
    // score and its config for the level
    long    score;                  // the current score
    long    scoreMin;               // the min score value the strip hold for the level
    long    scoreMax;               // the max score value the strip can hold for the level
    uint8_t scoreIncTimeRatio;      // increment time score ratio of the level
    uint8_t scoreDecTimeRatio;      // decrement time score ratio of the level
    long    scoreNextLevel;         // the score value needed for nex level
    long    scorePerLed;            // scores needed per led            

    // color and background
    uint8_t   leafColorHueOffset;   // the color h offset of the leaf
    uint8_t   backCompositionType;  // composition types are defined in CONFIG_BACK_TYPE_*
    uint8_t   backActiveColorV;     // color v value for active leds (if not used for fading the active is the default)
    uint8_t   backInactiveColorV;   // color v value for inactive leds
    uint8_t   backActiveColorS;     // color s value for active leds (if not used for fading the active is the default)
    uint8_t   backInactiveColorS;   // color s value for inactive leds
    
    // runners
    long runnerBaseTime;            // basetime of the runner started
    long runnerDiffTime;            // diff time (random) for the runner started
    long lastRunnerStartTime;       // timestamp the last runner was started
    RunnerCluster *runnerCluster;   // pointer to the runner cluster of the tree

    

    CRGB leds[CONFIG_MAX_LEDS_PER_LEAF];

    // methods of the LedLeaf
    LedLeaf(uint8_t leafID,
            int     numLeds,
            uint8_t sendPin,
            uint8_t sensePin,
            RunnerCluster *runnerCluster);
void setLevel(
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
    );
    void runCycle(uint8_t treeH, long now);
    void doSetup();
    void updateScore(bool sensed, long now);
    CHSV getLedBackColor(int ledIndex, long senseValue, uint8_t backH); 
    void reset();
    bool canLevelUp();
    bool canLevelDown();
};

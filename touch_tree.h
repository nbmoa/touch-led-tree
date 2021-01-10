struct TouchTree {
    long curCycleTimestamp;   // current cycles timestamp
    long lastCycleTimestamp;  // last cycle timestamp

    RunnerCluster runnerCluster;              // led runner cluster
    LedLeaf       ledLeaf[CONFIG_NUM_LEAFS];  // led leafs

    uint8_t treeLevel;
    long    levelStartTime;
    uint8_t treeH;
    uint8_t treeS;
    long    hueInterval;
    int     lastBrightnessUpdate;
    uint8_t brightness;

    TouchTree();
    void setup();
    void loop();
    void reset();
    void levelUp();
    void levelDown();
};

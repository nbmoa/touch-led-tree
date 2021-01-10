struct TouchTree {
    long curCycleTimestamp;   // current cycles timestamp
    long lastCycleTimestamp;  // last cycle timestamp

    RunnerCluster runnerCluster;              // led runner cluster
    LedLeaf       ledLeaf[CONFIG_NUM_LEAFS];  // led leafs

    long rainbowStartTime;  //
    uint8_t rainbowH;
    uint8_t rainbowS;

    TouchTree();
    void setup();
    void loop();
    void reset();
    void triggerRainbow();
};

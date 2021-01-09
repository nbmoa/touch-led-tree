struct TouchTree {
    long lastCycleTimestamp;

    RunnerCluster      runnerCluster;
    LedLeaf ledLeaf[CONFIG_NUM_LEAFS];

    long rainbowStartTime;
    uint8_t rainbowH;
    uint8_t rainbowS;

    TouchTree();
    void setup();
    void loop();
    void reset();
    void triggerRainbow();
};

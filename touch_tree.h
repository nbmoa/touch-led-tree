struct TouchTree {
    long curCycleTimestamp;   // current cycles timestamp
    long lastCycleTimestamp;  // last cycle timestamp

    RunnerCluster runnerCluster;              // led runner cluster
    LedLeaf       ledLeaf[CONFIG_NUM_LEAFS];  // led leafs

    uint8_t treeLevel;
    long    levelStartTime;
    uint8_t treeColorH;
    long    colorHInterval;
    uint8_t treeBrightness;
    long    treeBrightnessLastUpdate;

    TouchTree();
    void setup();
    void loop();
    void reset();
    void checkLevelChange();
    void setLevel(uint8_t newLevel);
    void treeCycle();
    void levelDown();
    void updateBrightness();
    
};

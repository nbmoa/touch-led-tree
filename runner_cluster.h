struct RunnerCluster {
    LedRunner runner[CONFIG_MAX_ACTIVE_RUNNERS];
    int executedRunners;

    void triggerRunner(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, long hueChangeInterval);
    CHSV getLedSprite(uint8_t leafID, int ledIndex);
    void update();
    void reset();
};

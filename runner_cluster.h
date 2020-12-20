
struct RunnerCluster {
    LedRunner runner[RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS];

    void triggerRunner(uint8_t stripID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime);
    CHSV getLedSprite(uint8_t stripID, int ledIndex);
    void update();
};


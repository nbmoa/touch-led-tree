#define RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS 4

struct RunnerCluster {
    LedRunner runner[RUNNER_CLUSTER_MAX_ACTIVE_RUNNERS];

    void triggerRunner(uint8_t leafID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime);
    CHSV getLedSprite(uint8_t leafID, int ledIndex);
    void update();
};


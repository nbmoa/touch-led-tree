struct RunnerCluster {
    LedRunner runner[CONFIG_MAX_ACTIVE_RUNNERS];
    int executedRunnerCnt;

    void triggerRunner(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, long hueChangeInterval, int fadeIn, int actDur, int fadeOut, long now);
    CHSV getLedSprite(uint8_t leafID, int ledIndex, long now);
    void update(long now);
    void reset();
};

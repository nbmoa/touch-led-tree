
struct LedRunner {
    bool active;
    
    uint8_t leafID;
    int numLeds;
    long runnerSpeed;
    long runnerLedSpeed;
    CHSV runnerColor;
    int glowNumLeds;
    
    uint8_t hueChange;     // amout of hue change in on interval
    long hueChangeInterval; // interval ms the change of the hue value is applied
    
    int activeLed;
    long hueLastChanged;
    long startTime;

    void start(uint8_t leafID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, int glowNumLeds);
    void updateHue();
    void updateActiveLed();
    void updateRunner();
    CHSV ledColor(int ledIndex);
};

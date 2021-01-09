
struct LedRunner {
    bool active;
    
    uint8_t leafID;
    int numLeds;
    long runnerSpeed;
    long runnerLedSpeed;
    uint8_t runnerColorH;
    int glowNumLeds;
    
    uint8_t hueChange;     // amout of hue change in on interval
    long hueChangeInterval; // interval ms the change of the hue value is applied
    
    long hueLastChanged;
    long startTime;

    void start(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, long hueChangeInterval, int glowNumLeds);
    void updateHue();
    void updateActiveLed();
    void updateRunner();
    CHSV getLedColor(int ledIndex, long now);
};

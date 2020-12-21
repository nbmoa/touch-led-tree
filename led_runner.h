
#define HSV_COLOR_TRANSPARENT CHSV{0,0,0}
struct LedRunner {
    bool active;
    
    uint8_t leafID;
    int numLeds;
    long runnerSpeed;
    CHSV runnerColor;
    long glowTime;
    uint8_t hueChange;     // amout of hue change in on interval
    long hueChangeInterval; // interval ms the change of the hue value is applied
    
    int activeLed;
    long hueLastChanged;
    long startTime;

    void start(uint8_t leafID, int numLeds, long runnerSpeed, CHSV runnerColor, uint8_t hueChange, long hueChangeInterval, long glowTime);
    void updateHue();
    void updateActiveLed();
    void updateRunner();
    CHSV ledColor(int ledIndex);
};

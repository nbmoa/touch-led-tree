struct LedRunner {
    // state of the runner
    bool    active;             // is the runner active
    long    startTime;          // startTime of the runner
    uint8_t leafID;             // the leaf the runner is associated with

    // the speed of the runner
    int  numLeds;               // amount of leds of the stip the runner is running for
    long runnerSpeed;           // time of the runner to run the complete strip
    long runnerLedSpeed;        // time for the runner to pass one led

    // color definition of the runner
    uint8_t runnerColorH;       // runners current hue value
    uint8_t hueChange;          // amout of hue change per interval
    int     hueChangeInterval;  // interval ms the change of the hue value is applied
    long    hueLastChanged;     // when did the hue last change

    // defines the shape of the runner
    int fadeInSpeed;
    int actLedDuration;
    int fadeOutSpeed;

    // methods of therunner
    void start(uint8_t leafID, int numLeds, long runnerSpeed, uint8_t runnerColorH, uint8_t hueChange, int hueChangeInterval, int fadeInSpeed, int actLedDuration, int fadeOutSpeed, long now);
    void updateRunner(long now);
    CHSV getLedColor(int ledIndex, long now);
};

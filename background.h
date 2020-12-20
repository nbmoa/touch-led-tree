
struct Background {
    int numLeds;         // Number of leds of the strip
    int ledActiveOffset; // the offset of the active led
    CHSV activeColor;    // LED active background color
    CHSV inactiveColor;  // LED inactive background color

    long allActiveTimeMs;   // time it takes till all leds are lit
    long allActiveSense;  // the sense value of the strip to light up all light, this is calculated baed on the allActiveTime
    
    Background(int numLeds, CHSV activeColor, CHSV inactiveColor, int ledActiveOffset);
    CHSV getLedBackColor(int ledIndex, long senseValue); 
};


struct Background {
    uint8_t activeColorV;    // LED active background color
    uint8_t inactiveColorV;  // LED inactive background color

    long timePerLed;    // sense value needed to light up all leds
    
    Background(uint8_t activeColorV,
               uint8_t inactiveColorV,
               long timePerLed);
    CHSV getLedBackColor(int ledIndex, long senseValue, uint8_t backH, uint8_t backS, uint8_t overlayV); 
};

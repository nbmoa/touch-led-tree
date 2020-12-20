
struct Background {
    CHSV activeColor;    // LED active background color
    CHSV inactiveColor;  // LED inactive background color

    long sensePerLed;    // sense value needed to light up all leds
    
    Background(CHSV activeColor,
               CHSV inactiveColor,
               long sensePerLed);
    CHSV getLedBackColor(int ledIndex, long senseValue); 
};

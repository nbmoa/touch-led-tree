struct SenseFilling {
    uint8_t incSenseMsRatio; // increment time ratio 
    uint8_t decSenseMsRatio; // decrement time ratio
    long    maxSense;        // the max sense value the strip can hold

    long accumulatedSense;   // currently accumulated Sense
    long lastSenseTimestamp; // last sense timetamp

    SenseFilling(uint8_t incSenseMsRatio, uint8_t decSenseMsRatio, long maxSense);
    void calculateCycle(bool sensed);
    long getCurrentSense();
};

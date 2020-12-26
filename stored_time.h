struct StoredTime {
    uint8_t incRatio; // increment time ratio 
    uint8_t decRatio; // decrement time ratio
    long    minTime;  // the min time value the strip hold
    long    maxTime;  // the max time value the strip can hold

    long storedTime;               // currently accumulated time
    long lastCycleTimestamp; // timestamp of last measurement

    StoredTime(uint8_t incRatio, uint8_t decRatio, long minTime, long maxTime);
    void update(bool sensed);
};

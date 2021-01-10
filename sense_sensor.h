struct SenseSensor {
    const uint8_t sendPin;  // send ping of the sensor
    const uint8_t sensePin; // sense/receive pin of the sensor
    bool enabled;           // state of the sensor
    long disabledSince;     // timestamp since the sensor is disabled (invalid if sensor is enabled

    CapacitiveSensor sensor;  // the sense sensor object

    // methods of the sense sensor
    SenseSensor(const uint8_t sendPin, const uint8_t sensePin);
    void doSetup();
    bool sense();
};

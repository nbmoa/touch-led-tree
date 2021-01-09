struct SenseSensor {
    const uint8_t sendPin;
    const uint8_t sensePin;
    bool active;
    long disabledSince;

    CapacitiveSensor sensor;

    SenseSensor(const uint8_t sendPin, const uint8_t sensePin);
    void doSetup();
    bool sense();
};

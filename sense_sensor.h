#define SENSE_SENSOR_MEASUREMENT_SAMPLES 10
#define SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS 10000 // if sense for a strip is disabled check every 1000 ms if the sense still times out
#define SENSE_SENSOR_DISABLE_TIMEOUT_MS 10 // timeout for the measurement that disables the sense measurement for SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS
#define SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD 1000
struct SenseSensor {
    const uint8_t sensePin;
    bool active;
    long disabledSince;

    CapacitiveSensor sensor;

    SenseSensor(const uint8_t sendPin, const uint8_t sensePin);
    void doSetup();
    bool sense();
};

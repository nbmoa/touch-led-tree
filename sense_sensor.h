#define SENSE_SENSOR_MEASUREMENT_SAMPLES 1
#define SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS CONFIG_SENSE_ENABLE_RETRY_INTERVAL_MS // if sense for a strip is disabled check every 1000 ms if the sense still times out
#define SENSE_SENSOR_DISABLE_TIMEOUT_MS 20 // timeout for the measurement that disables the sense measurement for SENSE_SENSOR_ENABLE_RETRY_INTERVAL_MS
#define SENSE_SENSOR_SENSE_ACTIVE_THREASHOLD CONFIG_SENSE_ACTIVE_THREASHOLD
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

/***********************************
 * AntPlus Bicycle Power Sensor example
 *
 * Creates a Bike Power Sensor, in
 * Crank Torque mode and transmits
 * openly with mocked data
 *
 * Example built for external radio
 *
 * Author Curtis Malainey
 ************************************/
#include <bluefruit.h>
#include <Arduino.h>
#include "ANT.h"
#include "ANTPLUS.h"
#include "ant_network_key.h"

#define BAUD_RATE 115200
#define CHANNEL_0 0

#define SENSOR_INTERRUPT_PIN    10
#define BUTTON_PIN              SW_PIN

#define NORMAL_BLINK_INTERVAL_MS    2000
#define DEMO_BLINK_INTERVAL_MS      200

// put in include/ant_network_key.h:
// const uint8_t NETWORK_KEY[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}; // get this from thisisant.com

// *************************************

ArduinoNativeAntWithCallbacks ant;
AntPlusRouter router;
ProfileBicyclePowerSensor bikePower(123, 0, ANTPLUS_BICYCLEPOWER_FLAGS_SENSORTYPE_POWERONLY);

void batteryStatusDataPageHandler(BatteryStatusMsg& msg, uintptr_t data);
void manufacturerIDDataPageHandler(ManufacturersInformationMsg& msg, uintptr_t data);
void productIDDataPageHandler(ProductInformationMsg& msg, uintptr_t data);
void powerOnlyDataPageHandler(BicyclePowerStandardPowerOnlyMsg& msg, uintptr_t data);
void wheelTorqueDataPageHandler(BicyclePowerStandardWheelTorqueMsg& msg, uintptr_t data);
void pedalSmoothnessDataPageHandler(BicyclePowerTorqueEffectivenessAndPedalSmoothnessMsg& msg, uintptr_t data);

void flash(unsigned count, uint32_t wait_ms = 250)
{
    for (unsigned i = 0; i < count; ++i)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(wait_ms);
        digitalWrite(LED_BUILTIN, LOW);
        delay(wait_ms);
    }
    delay(2 * wait_ms);
}


class WheelSensor
{
public:
    struct InterruptData
    {
        union
        {
            struct
            {
                volatile uint32_t revolution_count;
                volatile uint32_t event_time;
            };
            
            volatile uint64_t interrupt_data;
        };
    };
    static WheelSensor& instance()
    {
        static WheelSensor wheel_sensor;

        return wheel_sensor;
    }
    void begin(uint32_t sensor_pin)
    {
        pinMode(sensor_pin, INPUT_PULLUP);
        attachInterrupt(sensor_pin, pin_interrupt, FALLING);
    }
    uint16_t power_watts() const
    {
        if (!_received_first_interrupt)
        {
            return 0;
        }

        if (!_handled_first_request)
        {
            _handled_first_request = true;
            return 0;
        }
        InterruptData local_interrupt_data;

        noInterrupts();
        local_interrupt_data.interrupt_data = _interrupt_data.interrupt_data;
        interrupts();

        if (_last__interrupt_data.interrupt_data == local_interrupt_data.interrupt_data)
        {
            return 0;
        }

        auto revolution_count = local_interrupt_data.revolution_count - _last__interrupt_data.revolution_count;
        auto elapsed_time = local_interrupt_data.event_time - _last__interrupt_data.event_time;
        // Serial.printf("revs=%u, ms=%u\n", revolution_count, elapsed_time);

        auto revoltuions_per_second = static_cast<double>(1000000 * revolution_count) / static_cast<double>(elapsed_time);
        Serial.printf("RPS=%.4f\n", revoltuions_per_second);

        _last__interrupt_data.interrupt_data = local_interrupt_data.interrupt_data;

        auto power_watts = 0.0486 * pow(revoltuions_per_second, 1.86);
        Serial.printf("PWR=%.4f watts\n", power_watts);

        return static_cast<uint16_t>(power_watts);
    }
    void simulateInterrupt()
    {
        on_pin_interrupt();
    }
protected:
    static void pin_interrupt()
    {
        instance().on_pin_interrupt();
    }
    void on_pin_interrupt()
    {
        uint32_t now = micros();
        if (now - _interrupt_data.event_time > 3000)   // Debounce the interrupt. This is 91.44 kmh. Seems like a reasonable cap.
        {
            volatile InterruptData local_interrupt_data;
            local_interrupt_data.interrupt_data = _interrupt_data.interrupt_data;
            local_interrupt_data.revolution_count = _interrupt_data.interrupt_data + 1;
            local_interrupt_data.event_time = now;
            _interrupt_data.interrupt_data = local_interrupt_data.interrupt_data;
            _received_first_interrupt = true;
        }
    }
private:
    volatile InterruptData _interrupt_data{};
    volatile bool _received_first_interrupt = false;
    mutable InterruptData _last__interrupt_data{};
    mutable bool _handled_first_request = false;
};

volatile bool simulate_wheel_interrupt = false;
volatile uint32_t blink_interval_ms = NORMAL_BLINK_INTERVAL_MS;

void button_interrupt()
{
    simulate_wheel_interrupt = !simulate_wheel_interrupt;
    if (simulate_wheel_interrupt)
    {
        blink_interval_ms = DEMO_BLINK_INTERVAL_MS;
    }
    else
    {
        blink_interval_ms = NORMAL_BLINK_INTERVAL_MS;
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(BAUD_RATE);
    delay(100);
    Serial.println("Starting...");

    Bluefruit.begin();

    // *************************************

    router.setDriver(&ant); // never touch ant again
    router.setAntPlusNetworkKey(NETWORK_KEY);
    router.setProfile(CHANNEL_0, &bikePower);
    // Delay after initial setup to wait for user to connect on serial

    bikePower.createManufacturersInformationMsg(manufacturerIDDataPageHandler);
    bikePower.createProductInformationMsg(productIDDataPageHandler);
    bikePower.createBicyclePowerStandardPowerOnlyMsg(powerOnlyDataPageHandler);
    Serial.println("===========================");
    bikePower.begin();

    WheelSensor::instance().begin(SENSOR_INTERRUPT_PIN);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, button_interrupt, FALLING);

    Serial.println("Running...");
    flash(3, 200);

    // simulate_wheel_interrupt = true;
}

const uint32_t interruptIntervalMs = 20;    // Reports ~70 watts
volatile uint32_t lastInterruptTime = 0;

volatile uint32_t last_blink_time = 0;

void loop() {
    // call this frequently
    router.loop();
    auto now = millis();
    if (simulate_wheel_interrupt)
    {
        auto elapsedMs = now - lastInterruptTime;
        if (elapsedMs >= interruptIntervalMs)
        {
            WheelSensor::instance().simulateInterrupt();
            lastInterruptTime = now;
            // Serial.println(elapsedMs);
        }
    }
    if (now - last_blink_time >= blink_interval_ms)
    {
        digitalToggle(LED_BUILTIN);
        last_blink_time = now;
    }
}

void printDpMsg(int dp, const char* s) {
    Serial.print(millis());
    Serial.print(": ");
    Serial.print("Sending DataPage: ");
    Serial.print(dp);
    Serial.print(" - ");
    Serial.println(s);
}

void manufacturerIDDataPageHandler(ManufacturersInformationMsg& msg, uintptr_t data) {
    printDpMsg(80, "Manufacturers Information");
    msg.setHWRevision(1);
    msg.setManufacturerId(86);
    msg.setModelNumber(0);
}

void productIDDataPageHandler(ProductInformationMsg& msg, uintptr_t data) {
    printDpMsg(81, "Product Information");
    msg.setSWRevisionSupplemental(255);
    msg.setSWRevisionMain(17);
    msg.setSerialNumber(90);
}

void powerOnlyDataPageHandler(BicyclePowerStandardPowerOnlyMsg& msg, uintptr_t data) {
    static uint8_t eventCount = 0;
    static uint16_t accumulated_power = 0;

    printDpMsg(16, "Power Only");

    auto power_watts = WheelSensor::instance().power_watts();
    accumulated_power += power_watts;

    msg.setUpdateEventCount(++eventCount);
    msg.setCumulativePowerCount(accumulated_power);
    msg.setInstantaneousPowerCount(power_watts);
}

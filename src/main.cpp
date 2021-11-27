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
#define antSerial Serial2

// put in include/ant_network_key.h:
// const uint8_t NETWORK_KEY[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}; // get this from thisisant.com

// *************************************

ArduinoNativeAntWithCallbacks ant;
AntPlusRouter router;
ProfileBicyclePowerSensor bikePower(123, 0, ANTPLUS_BICYCLEPOWER_FLAGS_SENSORTYPE_TORQUECRANK);

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

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(BAUD_RATE);
    flash(2);
    delay(500);
    Serial.println("Running");

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

    Serial.println("Running...");
    flash(3);
}

void loop() {
    // call this frequently
    router.loop();
}

void printDpMsg(int dp, const char* s) {
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

    printDpMsg(16, "Power Only");
    msg.setUpdateEventCount(eventCount++);
    msg.setCumulativePowerCount(eventCount * 3);
    msg.setInstantaneousPowerCount(eventCount * 2);
}

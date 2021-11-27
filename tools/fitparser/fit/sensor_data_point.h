#pragma once

#include <map>

#include <stdint.h>


class SensorDataPoint
{
    public:
        enum class FieldType
        {
            // Speed and cadence sensor:
            WheelRevolutionCount,
            WheelRevolutionEventTime,
            CrankRevolutionCount,
            CrankRevolutionEvtTime,
            // Power sensor
            UpdateEventCount,
            InstantaneousPower,
            AccumulatedPower,
        };
        SensorDataPoint(uint32_t timestamp_ms);
        uint32_t timestamp_ms() const;
        bool has_value(FieldType field_type) const;
        unsigned get_value(FieldType field_type) const;
        void store_value(FieldType field_type, unsigned value);
    private:
        uint32_t _timestamp_ms;
        std::map<FieldType, unsigned> _fields;
};
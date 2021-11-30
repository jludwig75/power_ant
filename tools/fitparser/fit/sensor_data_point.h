#pragma once

#include <cassert>
#include <map>

#include <stdint.h>


template<typename _T>
class DataPoint
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
            // Calculated
            SpeedRevolutionsPerSecond,
            CrankRevolutionsPerSecond,
        };
        DataPoint(uint32_t timestamp_ms)
            : _timestamp_ms(timestamp_ms)
        {

        }
        uint32_t timestamp_ms() const
        {
            return _timestamp_ms;
        }
        bool has_value(FieldType field_type) const
        {
            return _fields.find(field_type) != _fields.end();
        }
        _T get_value(FieldType field_type) const
        {
            auto it = _fields.find(field_type);
            assert(it != _fields.end());

            return it->second;
        }
        void store_value(FieldType field_type, _T value)
        {
            _fields[field_type] = value;
        }
        bool empty() const
        {
            return _fields.empty();
        }
    private:
        uint32_t _timestamp_ms;
        std::map<FieldType, _T> _fields;
};

typedef DataPoint<uint16_t> SensorDataPoint;

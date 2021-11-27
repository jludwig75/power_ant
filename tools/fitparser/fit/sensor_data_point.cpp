#include "sensor_data_point.h"

#include <cassert>


SensorDataPoint::SensorDataPoint(uint32_t timestamp_ms)
    : _timestamp_ms(timestamp_ms)
{

}

uint32_t SensorDataPoint::timestamp_ms() const
{
    return _timestamp_ms;
}

bool SensorDataPoint::has_value(FieldType field_type) const
{
    return _fields.find(field_type) != _fields.end();

}

unsigned SensorDataPoint::get_value(FieldType field_type) const
{
    auto it = _fields.find(field_type);
    assert(it != _fields.end());

    return it->second;
}

void SensorDataPoint::store_value(FieldType field_type, unsigned value)
{
    _fields[field_type] = value;
}

#pragma once

#include "ant_page.h"

#include <algorithm>
#include <memory>


struct SpeedAndCadencePage : public AntCommonPage
{
    static std::shared_ptr<AntCommonPage> parse_page(std::array<uint8_t, 8>& page_data);

    SpeedAndCadencePage(const std::array<uint8_t, 8>& data)
        : AntCommonPage(data)
    {
    }

    uint16_t cadence_event_time() const
    {
        return get_u16(0);
    }

    uint16_t cadence_revolution_count() const
    {
        return get_u16(2);
    }

    uint16_t speed_event_time() const
    {
        return get_u16(4);
    }

    uint16_t speed_revolutution_count() const
    {
        return get_u16(6);
    }

    std::string dump() const override
    {
        std::stringstream ss;
        ss << "Spd & cdnc: cadence evt time=" << cadence_event_time() <<
            ", cadence rev cnt=" << cadence_revolution_count() <<
            ", spd evt time=" << speed_event_time() <<
            ", spd rev count=" << speed_revolutution_count();

        return ss.str();// + " - " + AntCommonPage::dump();
    }

    void populate_data_point(const std::vector<SensorDataPoint::FieldType>& requested_fields, SensorDataPoint& data_point) override
    {
        if (std::find(requested_fields.begin(), requested_fields.end(), SensorDataPoint::FieldType::CrankRevolutionCount) != requested_fields.end())
        {
            data_point.store_value(SensorDataPoint::FieldType::CrankRevolutionCount, cadence_revolution_count());
        }

        if (std::find(requested_fields.begin(), requested_fields.end(), SensorDataPoint::FieldType::CrankRevolutionEvtTime) != requested_fields.end())
        {
            data_point.store_value(SensorDataPoint::FieldType::CrankRevolutionEvtTime, cadence_event_time());
        }

        if (std::find(requested_fields.begin(), requested_fields.end(), SensorDataPoint::FieldType::WheelRevolutionCount) != requested_fields.end())
        {
            data_point.store_value(SensorDataPoint::FieldType::WheelRevolutionCount, speed_revolutution_count());
        }

        if (std::find(requested_fields.begin(), requested_fields.end(), SensorDataPoint::FieldType::WheelRevolutionEventTime) != requested_fields.end())
        {
            data_point.store_value(SensorDataPoint::FieldType::WheelRevolutionEventTime, speed_event_time());
        }
    }
};

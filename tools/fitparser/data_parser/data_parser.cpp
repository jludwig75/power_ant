#include <iostream>
#include <vector>

#include "data_log.h"

#include <fit/sensor_data_point.h>
#include <fit/bike_power_page.h>
#include <fit/speed_and_cadence_page.h>


void dump_csv(std::ostream& out,
    const std::vector<SensorDataPoint::FieldType>& requested_fields,
    const std::vector<SensorDataPoint> &data_points)
{
    out << "time(ms)";

    for (const auto& field_type : requested_fields)
    {
        switch (field_type)
        {
            case SensorDataPoint::FieldType::WheelRevolutionCount:
                out << ",wheel_rev_count";
                break;
            case SensorDataPoint::FieldType::WheelRevolutionEventTime:
                out << ",wheel_rev_time";
                break;
            case SensorDataPoint::FieldType::CrankRevolutionCount:
                out << ",crank_rev_count";
                break;
            case SensorDataPoint::FieldType::CrankRevolutionEvtTime:
                out << ",crank_rev_time";
                break;
            case SensorDataPoint::FieldType::UpdateEventCount:
                out << ",updated_event_count";
                break;
            case SensorDataPoint::FieldType::InstantaneousPower:
                out << ",inst_power(watts)";
                break;
            case SensorDataPoint::FieldType::AccumulatedPower:
                out << ",acc_power(watts)";
                break;
        }
    }
    out << "\n";

    for (const auto& data_point : data_points)
    {
        out << data_point.timestamp_ms();
        for (const auto& field_type : requested_fields)
        {
            out << ",";
            if (data_point.has_value(field_type))
            {
                out << data_point.get_value(field_type);
            }
        }
        out << "\n";
    }
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "You must specify a log file name\n");
        return -1;
    }

    DataLog data_log(argv[1]);
    
    uint64_t timestamp;
    DataLog::SensorType sensor_type;
    PageData ant_page;
    std::vector<SensorDataPoint> data_points;
    std::vector<SensorDataPoint::FieldType> requested_fields = {SensorDataPoint::FieldType::WheelRevolutionEventTime,
        SensorDataPoint::FieldType::WheelRevolutionCount,
        SensorDataPoint::FieldType::UpdateEventCount,
        SensorDataPoint::FieldType::InstantaneousPower};
    while (data_log.read_page(timestamp, sensor_type, ant_page))
    {
        if (sensor_type == DataLog::SensorType::Power)
        {
            auto page = BicyclePowerPage::parse_page(ant_page);
            // std::cout << page->dump() << "\n";
            SensorDataPoint data_point(timestamp);
            page->populate_data_point(requested_fields, data_point);
            data_points.push_back(std::move(data_point));
        }
        else if (sensor_type == DataLog::SensorType::SpeedAndCadence)
        {
            auto page = SpeedAndCadencePage::parse_page(ant_page);
            // std::cout << page->dump() << "\n";
            SensorDataPoint data_point(timestamp);
            page->populate_data_point(requested_fields, data_point);
            data_points.push_back(std::move(data_point));
        }
        else
        {
            printf("Unsupported device type: %u\n", static_cast<unsigned>(sensor_type));
        }
    }

    dump_csv(std::cout, requested_fields, data_points);

    return 0;
}
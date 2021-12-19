#include <iostream>
#include <vector>

#include "data_log.h"
#include "data_series.h"

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
        if (data_point.empty())
        {
            continue;
        }

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

void parse_data_set(const std::string& data_log_file_name,
    const std::vector<SensorDataPoint::FieldType>& requested_fields,
    std::vector<SensorDataPoint>& data_points)
{
    DataLog data_log(data_log_file_name);
    
    uint64_t start_time{UINT64_MAX};
    uint64_t timestamp;
    DataLog::SensorType sensor_type;
    PageData ant_page;
    while (data_log.read_page(timestamp, sensor_type, ant_page))
    {
        if (start_time == UINT64_MAX)
        {
            start_time = timestamp;
        }
        float time_sec = static_cast<float>(timestamp - start_time) / 1000.0;
        if (sensor_type == DataLog::SensorType::Power)
        {
            auto page = BicyclePowerPage::parse_page(ant_page);
            // std::cout << std::fixed << time_sec << ": " << page->dump() << "\n";
            SensorDataPoint data_point(timestamp);
            page->populate_data_point(requested_fields, data_point);
            if (!data_point.empty())
            {
                data_points.push_back(std::move(data_point));
            }
        }
        else if (sensor_type == DataLog::SensorType::SpeedAndCadence)
        {
            auto page = SpeedAndCadencePage::parse_page(ant_page);
            // std::cout << std::fixed << time_sec << ": " << page->dump() << "\n";
            SensorDataPoint data_point(timestamp);
            page->populate_data_point(requested_fields, data_point);
            if (!data_point.empty())
            {
                data_points.push_back(std::move(data_point));
            }
        }
    }
}


void dump_raw_fields(const std::string& data_log_file_name)
{
    std::vector<SensorDataPoint> data_points;
    std::vector<SensorDataPoint::FieldType> requested_fields = {
            // SensorDataPoint::FieldType::WheelRevolutionEventTime,
            // SensorDataPoint::FieldType::WheelRevolutionCount,
            SensorDataPoint::FieldType::UpdateEventCount,
            SensorDataPoint::FieldType::AccumulatedPower,
            SensorDataPoint::FieldType::InstantaneousPower
        };
    parse_data_set(data_log_file_name, requested_fields, data_points);
    dump_csv(std::cout, requested_fields, data_points);
}

typedef DataPoint<float> SpeedDataPoint;

void correlate_speed_and_power(const std::string& data_log_file_name)
{
    std::vector<SensorDataPoint> speed_data_points;
    parse_data_set(data_log_file_name,
        {SensorDataPoint::FieldType::WheelRevolutionEventTime,
            SensorDataPoint::FieldType::WheelRevolutionCount},
        speed_data_points);

    // Calculate speeds
    const SensorDataPoint* last_data_point = nullptr;
    std::vector<SpeedDataPoint> revs_data_points;
    for (const auto& data_point : speed_data_points)
    {
        if (last_data_point != nullptr)
        {
            uint16_t last_time_event = last_data_point->get_value(SensorDataPoint::FieldType::WheelRevolutionEventTime);
            uint16_t last_revolution_count = last_data_point->get_value(SensorDataPoint::FieldType::WheelRevolutionCount);
            uint16_t current_time_event = data_point.get_value(SensorDataPoint::FieldType::WheelRevolutionEventTime);
            uint16_t current_revolution_count = data_point.get_value(SensorDataPoint::FieldType::WheelRevolutionCount);

            uint16_t elapsed_time_units = current_time_event - last_time_event;
            uint16_t elapsed_revolutions = current_revolution_count - last_revolution_count;

            float revolutions_per_second = 0;
            if (current_time_event != last_time_event)
            {
                auto elapsed_time_sec = static_cast<double>(elapsed_time_units) / 1024.0;
                revolutions_per_second = static_cast<float>(static_cast<double>(elapsed_revolutions) / elapsed_time_sec);
                // std::cout << "(" << current_revolution_count << " - " << last_revolution_count << ") / " <<
                //             "(" << current_time_event << " - " << last_time_event << ") = " <<
                //             elapsed_revolutions << " / " << elapsed_time_units << " = " <<
                //             revolutions_per_second << std::endl;
            }
            // else
            // {
            //     std::cout << 0.0 << std::endl;
            // }

            SpeedDataPoint speed_data_point(data_point.timestamp_ms());
            speed_data_point.store_value(SpeedDataPoint::FieldType::SpeedRevolutionsPerSecond, revolutions_per_second);
            revs_data_points.push_back(std::move(speed_data_point));
        }

        last_data_point = &data_point;
    }
    DataSeries<float> revs_data_series(std::move(revs_data_points));

    // Extract power events
    std::vector<SensorDataPoint> power_data_points;
    parse_data_set(data_log_file_name, {SensorDataPoint::FieldType::InstantaneousPower}, power_data_points);

    // Match speed and power events
    std::cout << "revolutions_per_second,power(watts)" << std::endl;
    std::vector<std::pair<float, unsigned>> revs_to_power_map;
    for (const auto& power_data_point : power_data_points)
    {
        auto power_timestmap_ms = power_data_point.timestamp_ms();
        // Find a matching revs datapoint
        auto revs_data_point = revs_data_series.get_latest_data_point_up_to(power_timestmap_ms);
        if (revs_data_point != nullptr && power_timestmap_ms - revs_data_point->timestamp_ms() <= 250)
        {
            std::cout << revs_data_point->get_value(SpeedDataPoint::FieldType::SpeedRevolutionsPerSecond) <<
                "," << power_data_point.get_value(SensorDataPoint::FieldType::InstantaneousPower) << std::endl;
        }
    }

    // Dump matched speed and power events to CSV
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "You must specify a log file name\n");
        return -1;
    }

    dump_raw_fields(argv[1]);
    // correlate_speed_and_power(argv[1]);


    return 0;
}
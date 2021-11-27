#include <iostream>

#include "data_log.h"

#include <fit/bike_power_page.h>
#include <fit/speed_and_cadence_page.h>


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
    while (data_log.read_page(timestamp, sensor_type, ant_page))
    {
        if (sensor_type == DataLog::SensorType::Power)
        {
            auto page = BicyclePowerPage::parse_page(ant_page);
            std::cout << page->dump() << "\n";
        }
        else if (sensor_type == DataLog::SensorType::SpeedAndCadence)
        {
            auto page = SpeedAndCadencePage::parse_page(ant_page);
            std::cout << page->dump() << "\n";
        }
        else
        {
            printf("Unsupported device type: %u\n", static_cast<unsigned>(sensor_type));
        }
    }

    return 0;
}
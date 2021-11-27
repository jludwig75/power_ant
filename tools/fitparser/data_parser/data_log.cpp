#include "data_log.h"



struct __attribute__((__packed__)) DataLogEntry
{
    uint64_t timestamp;
    uint32_t sensor_type;
    PageData fit_data;
};

static_assert(sizeof(DataLogEntry) == 8 + 4 + 8);


DataLog::DataLog(const std::string& file_name)
{
    _file = std::make_unique<std::ifstream>(file_name, std::ios::binary);
    if (!_file->good())
    {
        throw Exception("Error opening log file \"" + file_name + "\"");
    }
}

bool DataLog::read_page(uint64_t timestamp_ms, SensorType& sensor_type, PageData& page)
{
    DataLogEntry entry;
    _file->read(reinterpret_cast<char *>(&entry), sizeof(entry));

    timestamp_ms = entry.timestamp;
    sensor_type = static_cast<SensorType>(entry.sensor_type);
    // printf("%02X:\n", entry.fit_data.data[0]);
    page = entry.fit_data;

    // printf("%lu, %u\n", timestamp_ms, static_cast<unsigned>(sensor_type));

    return _file->good();
}

#pragma once

#include <fstream>
#include <memory>
#include <string>

#include <fit/ant_page.h>



typedef std::array<uint8_t, 8> PageData;

static_assert(sizeof(PageData) == 8);


class DataLog
{
public:
    DataLog(const std::string& file_name);

    enum class SensorType
    {
        Power = 11,
        SpeedAndCadence = 121,
    };

    class Exception : public std::exception
    {
        public:
            Exception(const std::string& error_message)
                : _error_message(error_message)
            {
            }
            const char* what() const throw()
            {
                return _error_message.c_str();
            }
        private:
            std::string _error_message;
    };
    bool read_page(uint64_t& timestamp_ms, SensorType& sensor_type, PageData& page);
private:
    std::unique_ptr<std::ifstream> _file;
};
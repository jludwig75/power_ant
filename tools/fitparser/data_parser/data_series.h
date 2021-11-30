#include <fit/sensor_data_point.h>

#include <iostream>
#include <vector>


template<typename _T>
class DataSeries
{
    public:
        DataSeries(const std::vector<DataPoint<_T>>& data_points)
            : _data_points(data_points)
        {
        }
        const DataPoint<_T>* get_latest_data_point_up_to(uint32_t timestamp_ms) const
        {
            const DataPoint<_T>* latest_data_point_before = nullptr;

            for (const auto& data_point : _data_points)
            {
                if (data_point.timestamp_ms() > timestamp_ms)
                {
                    break;
                }

                latest_data_point_before = &data_point;
            }

            return latest_data_point_before;
        }

    private:
        std::vector<DataPoint<_T>> _data_points;
};

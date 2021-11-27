#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <vector>

#include "sensor_data_point.h"


struct AntCommonPage
{
    AntCommonPage(const std::array<uint8_t, 8>& data)
        : data(data)
    {
        
    }
    std::array<uint8_t, 8> data;

    uint8_t page_number() const;

    uint16_t get_u16(unsigned offset) const;

    uint32_t get_u32(unsigned offset) const;

    virtual std::string dump() const
    {
        std::stringstream ss;
        for (auto i = 0; i < sizeof(data); ++i)
        {
            if (i > 0)
            {
                ss << ", ";
            }
            unsigned field = data[i];
            ss << "[0x" << std::hex << std::setfill('0') << std::setw(2) << field << "]";
        }

        return ss.str();
    }

    virtual void populate_data_point(const std::vector<SensorDataPoint::FieldType>& requested_fields, SensorDataPoint& data_point)
    {
        // Nothing to populate
    }
};

struct ManufacturerInformationPage : public AntCommonPage
{
    ManufacturerInformationPage(const std::array<uint8_t, 8>& data)
        : AntCommonPage(data)
    {
    }

    uint8_t get_hw_revision() const
    {
        return data[3];
    }

    uint16_t get_manfacturer_id() const
    {
        return get_u16(4);
    }

    uint16_t get_model_number() const
    {
        return get_u16(6);
    }

    std::string dump() const override
    {
        std::stringstream ss;
        ss << "Mfg Info: hw rev=" << static_cast<unsigned>(get_hw_revision()) <<
            ", mfg ID=" << get_manfacturer_id() <<
            ", model num: " << get_model_number();

        return ss.str();// + " - " + AntCommonPage::dump();
    }
};


struct ProductInformationPage : public AntCommonPage
{
    ProductInformationPage(const std::array<uint8_t, 8>& data)
        : AntCommonPage(data)
    {
    }

    uint8_t get_supplemental_sw_revision() const
    {
        return data[2];
    }

    uint8_t get_sw_revision() const
    {
        return data[3];
    }

    uint32_t get_serial_number() const
    {
        return get_u32(4);
    }

    std::string dump() const override
    {
        std::stringstream ss;
        ss << "Product Info: sw rev=" << static_cast<unsigned>(get_sw_revision()) <<
            ", sup sw rev=" << static_cast<unsigned>(get_supplemental_sw_revision()) <<
            ", serial num: " << get_serial_number();
        
        return ss.str();// + " - " + AntCommonPage::dump();
    }
};


struct BatteryStatusPage : public AntCommonPage
{
    BatteryStatusPage(const std::array<uint8_t, 8>& data)
        : AntCommonPage(data)
    {
    }

    uint8_t battery_identifier() const
    {
        return data[2];
    }

    uint32_t cumulative_opertating_time() const
    {
        return 0xFFF & get_u32(3);
    }

    uint8_t fractional_battery_voltage() const
    {
        return data[6];
    }

    uint8_t descriptive_bit_field() const
    {
        return data[7];
    }

    std::string dump() const override
    {
        std::stringstream ss;
        ss << "Batt Status: batt ID=" << static_cast<unsigned>(battery_identifier()) <<
            ", cum op time=" << cumulative_opertating_time() <<
            ", fractnl batt=" << static_cast<unsigned>(fractional_battery_voltage()) << "/255" <<
            ", descriptive bit field=0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(descriptive_bit_field());
        
        return ss.str();// + " - " + AntCommonPage::dump();
    }
};
#pragma once

#include "ant_page.h"

#include <memory>


struct BicyclePowerPage : public AntCommonPage
{
    BicyclePowerPage(const std::array<uint8_t, 8>& data)
        : AntCommonPage(data)
    {
    }
    static std::shared_ptr<AntCommonPage> parse_page(std::array<uint8_t, 8>& page_data);
};

struct StandardPowerOnlyPage : public BicyclePowerPage
{
    StandardPowerOnlyPage(const std::array<uint8_t, 8>& data)
        : BicyclePowerPage(data)
    {
    }
    uint8_t update_event_count() const
    {
        return data[1];
    }

    uint8_t pedal_power() const
    {
        return data[2];
    }

    uint8_t instantaneous_cadence() const
    {
        return data[3];
    }

    uint16_t accumulated_power() const
    {
        return get_u16(4);
    }

    uint16_t instantaneous_power() const
    {
        return get_u16(6);
    }

    std::string dump() const override
    {
        std::stringstream ss;
        ss << "Std Pwr Only: updt evt count=" << static_cast<unsigned>(update_event_count()) <<
            ", pedal pwr=" << static_cast<unsigned>(pedal_power()) <<
            ", inst cadence=" << static_cast<unsigned>(instantaneous_cadence()) <<
            ", acc pwr=" << accumulated_power() <<
            ", inst pwr=" << instantaneous_power();

        return ss.str();// + " - " + AntCommonPage::dump();
    }
};
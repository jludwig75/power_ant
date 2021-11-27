#pragma once

#include "ant_page.h"

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
};

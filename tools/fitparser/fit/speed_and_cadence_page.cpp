#include "speed_and_cadence_page.h"


std::shared_ptr<AntCommonPage> SpeedAndCadencePage::parse_page(std::array<uint8_t, 8>& page_data)
{
    return std::make_shared<SpeedAndCadencePage>(page_data);
}
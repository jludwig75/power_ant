#include "ant_page.h"


uint8_t AntCommonPage::page_number() const
{
    return data[0];
}

uint16_t AntCommonPage::get_u16(unsigned offset) const
{
    assert(offset < sizeof(*this) - 1);

    return *reinterpret_cast<const uint16_t*>(&data[offset]);
}

uint32_t AntCommonPage::get_u32(unsigned offset) const
{
    assert(offset < sizeof(*this) - 3);

    return *reinterpret_cast<const uint32_t*>(&data[offset]);
}

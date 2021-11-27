#include "bike_power_page.h"


std::shared_ptr<AntCommonPage> BicyclePowerPage::parse_page(std::array<uint8_t, 8>& page_data)
{
    uint8_t page_number = page_data[0];

    switch (page_number)
    {
        case 0x10:
            return std::make_shared<StandardPowerOnlyPage>(page_data);
        case 0x50:
            return std::make_shared<ManufacturerInformationPage>(page_data);
        case 0x51:
            return std::make_shared<ProductInformationPage>(page_data);
        case 0x52:
            return std::make_shared<BatteryStatusPage>(page_data);
        default:
            return std::make_shared<AntCommonPage>(page_data);
    }
}
#include "options.h"
#include "whm_1.h"
#include "comm.h"

energy_ch_type energy_group_to_ch_type(uint8_t grp)
{
    switch (grp)
    {
    case 1:
        return eChDeliAct;  // 수전 유효
    case 2:
        return eChReceiAct;  // 송전 유효
    case 5:
        return eChDLagReact;  // 수전 지상 무효
    case 6:
        return eChRLeadReact;  // 송전 진상 무효
    case 7:
        return eChRLagReact;  // 송전 지상 무효
    case 8:
        return eChDLeadReact;  // 수전 진상 무효
    case 9:
        return eChDeliApp;  // 수전 피상
    case 10:
        return eChReceiApp;  // 송전 피상
    }
    return eChDeliAct;
}

rate_type energy_group_to_tariff(uint8_t grp)
{
    if (grp < numRates)
    {
        if (grp == 0)
            return eTrate;
        return (rate_type)(grp - 1);
    }
    return eTrate;
}

demand_ch_type energy_group_to_dmch_type(uint8_t grp)
{
    switch (grp)
    {
    case 1:
        return eDmChDeliAct;
    case 9:
        return eDmChDeliApp;
    case 2:
        return eDmChReceiAct;
    case 10:
        return eDmChReceiApp;
    }
    return eDmChDeliAct;
}

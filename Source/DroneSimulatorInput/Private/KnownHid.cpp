#include "DroneSimulatorInput/Public/KnownHid.h"


EKnownHidVendor UKnownHidLibrary::parse_vendor_id(int32 vendor_id)
{
    switch (vendor_id)
    {
    case 0x2CA3:
        return EKnownHidVendor::DJI;
    case 0x1209:
        return EKnownHidVendor::PidCodes;
    case 0x19F7:
        return EKnownHidVendor::Rode;
    case 0x045E:
        return EKnownHidVendor::Microsoft;
    case 0x046D:
        return EKnownHidVendor::Logitech;
    case 0x1532:
        return EKnownHidVendor::Razer;
    case 0x054C:
        return EKnownHidVendor::Sony;
    default:
        return EKnownHidVendor::Unknown;
    }
}

EKnownHidProduct UKnownHidLibrary::parse_product_id(const EKnownHidVendor& vendor_known, int32 product_id)
{
    switch (vendor_known)
    {
    case EKnownHidVendor::PidCodes:
        switch (product_id)
        {
        case 0x4F54:
            return EKnownHidProduct::OpenTx;
        default:
            return EKnownHidProduct::Unknown;
        }

    case EKnownHidVendor::DJI:
        switch (product_id)
        {
        case 0x1020:
            return EKnownHidProduct::DJI_FpvRc2;
        default:
            return EKnownHidProduct::Unknown;
        }

    case EKnownHidVendor::Microsoft:
        switch (product_id)
        {
        case 0x02FF:
            return EKnownHidProduct::MS_XboxController;
        default:
            return EKnownHidProduct::Unknown;
        }

    case EKnownHidVendor::Sony:
        switch (product_id)
        {
        case 0x0CE6:
            return EKnownHidProduct::Sony_Ps5Controller_Wired;
        case 0x0DF2:
            return EKnownHidProduct::Sony_Ps5Controller_BT;
        default:
            return EKnownHidProduct::Unknown;
        }

    default:
        return EKnownHidProduct::Unknown;
    }
}

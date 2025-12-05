#include "HIDDevice.h"

#if PLATFORM_WINDOWS

FWindowsHIDDevice::FWindowsHIDDevice(int32 InInternalId, const FString& InDevicePath)
    : internal_id(InInternalId)
    , device_path(InDevicePath)
    , device_handle(INVALID_HANDLE_VALUE)
    , preparsed_data(nullptr)
{
}

FWindowsHIDDevice::~FWindowsHIDDevice()
{
    close();
}

bool FWindowsHIDDevice::open()
{
    device_handle = CreateFile(
        *device_path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        nullptr
    );

    if (device_handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // Get Product Name
    wchar_t buffer[128];
    if (HidD_GetProductString(device_handle, buffer, sizeof(buffer)))
    {
        product_name = buffer;
    }
    else
    {
        product_name = FString::Printf(TEXT("HID Device %d"), internal_id);
    }

    // Parse Caps
    if (HidD_GetPreparsedData(device_handle, &preparsed_data))
    {
        HidP_GetCaps(preparsed_data, &capabilities);
        parse_capabilities();

        // Prepare Input Buffer
        input_report_buffer.SetNumZeroed(capabilities.InputReportByteLength);
    }

    // Setup Overlapped
    FMemory::Memzero(&overlapped, sizeof(OVERLAPPED));
    overlapped.hEvent = CreateEvent(nullptr, 1, 0, nullptr); // ManualReset=TRUE, InitialState=FALSE

    // Start first read
    issue_read();

    return true;
}

void FWindowsHIDDevice::close()
{
    if (overlapped.hEvent != INVALID_HANDLE_VALUE)
    {
        CancelIo(device_handle);
        CloseHandle(overlapped.hEvent);
        overlapped.hEvent = INVALID_HANDLE_VALUE;
    }

    if (preparsed_data)
    {
        HidD_FreePreparsedData(preparsed_data);
        preparsed_data = nullptr;
    }

    if (device_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(device_handle);
        device_handle = INVALID_HANDLE_VALUE;
    }
}

void FWindowsHIDDevice::parse_capabilities()
{
    if (!preparsed_data) return;

    // Values (Axes)
    USHORT value_caps_length = capabilities.NumberInputValueCaps;
    if (value_caps_length > 0)
    {
        value_capabilities.SetNum(value_caps_length);
        HidP_GetValueCaps(HidP_Input, value_capabilities.GetData(), &value_caps_length, preparsed_data);
    }

    // Buttons
    USHORT button_caps_length = capabilities.NumberInputButtonCaps;
    if (button_caps_length > 0)
    {
        button_capabilities.SetNum(button_caps_length);
        HidP_GetButtonCaps(HidP_Input, button_capabilities.GetData(), &button_caps_length, preparsed_data);
    }
}

EHIDPollStatus FWindowsHIDDevice::poll(FDroneInputDevice& out_device_data)
{
    if (this->device_handle == INVALID_HANDLE_VALUE) return EHIDPollStatus::Error;

    // Check if read completed
    DWORD wait_result = WaitForSingleObject(overlapped.hEvent, 0);
    if (wait_result == WAIT_OBJECT_0)
    {
        DWORD bytes_read = 0;
        if (GetOverlappedResult(this->device_handle, &this->overlapped, &bytes_read, 0)) // FALSE -> 0
        {
            if (bytes_read > 0)
            {
                // Process Data
                out_device_data.device_id = this->internal_id;
                out_device_data.device_name = this->product_name;
                out_device_data.device_uid = this->device_path;

                // Parse Values
                for (const auto& capability : this->value_capabilities)
                {
                    ULONG value = 0;
                    if (HidP_GetUsageValue(HidP_Input, capability.UsagePage, 0, capability.Range.UsageMin, &value, preparsed_data, (PCHAR)input_report_buffer.GetData(), input_report_buffer.Num()) == HIDP_STATUS_SUCCESS)
                    {
                        // Normalize
                        float normalized = 0.0f;
                        int32 min = capability.LogicalMin;
                        int32 max = capability.LogicalMax;
                        int32 range = max - min;

                        int32 raw_value = (int32)value;

                        // If LogicalMin is negative, the value is likely signed.
                        // HidP_GetUsageValue returns ULONG (unsigned 32-bit), so we must sign-extend manually if the bit size is smaller.
                        if (min < 0)
                        {
                            if (capability.BitSize == 16)
                            {
                                raw_value = (int16)value;
                            }
                            else if (capability.BitSize == 8)
                            {
                                raw_value = (int8)value;
                            }
                        }

                        if (range != 0)
                        {
                            normalized = (float)(raw_value - min) / (float)range;
                            // Map to -1..1 to match standard Gamepad input
                            normalized = (normalized * 2.0f) - 1.0f;
                        }

                        // Name: UsagePage_Usage (e.g. GenericDesktop_X)
                        FString axis_name = FString::Printf(TEXT("HID_%04X_%04X"), capability.UsagePage, capability.Range.UsageMin);
                        out_device_data.raw_axes.Add(FName(*axis_name), normalized);
                        out_device_data.raw_int_axes.Add(FName(*axis_name), raw_value);
                    }
                }

                // Issue next read
                ResetEvent(overlapped.hEvent);
                issue_read();
                return EHIDPollStatus::Success;
            }
        }
        else
        {
            // GetOverlappedResult failed. This usually means the device was disconnected.
            DWORD error = GetLastError();
            if (error == ERROR_DEVICE_NOT_CONNECTED || error == ERROR_GEN_FAILURE)
            {
                return EHIDPollStatus::Error;
            }
        }
    }
    else if (wait_result == WAIT_FAILED || wait_result == WAIT_ABANDONED)
    {
        return EHIDPollStatus::Error;
    }

    return EHIDPollStatus::NoData;
}


void FWindowsHIDDevice::issue_read()
{
    if (device_handle != INVALID_HANDLE_VALUE)
    {
        FMemory::Memzero(input_report_buffer.GetData(), input_report_buffer.Num());
        ReadFile(device_handle, input_report_buffer.GetData(), input_report_buffer.Num(), nullptr, &overlapped);
    }
}

#else

FWindowsHIDDevice::FWindowsHIDDevice(int32 InInternalId, const FString& InDevicePath) {}
FWindowsHIDDevice::~FWindowsHIDDevice() {}
bool FWindowsHIDDevice::Open() { return false; }
void FWindowsHIDDevice::Close() {}
EHIDPollStatus FWindowsHIDDevice::poll(FDroneInputDevice& out_device_data) { return EHIDPollStatus::NoData; }

#endif

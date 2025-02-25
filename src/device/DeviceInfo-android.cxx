#if defined(__ANDROID__)
#include "DeviceInfo.hxx"

#include <cstddef>
#include <cstdint>

#include <sys/system_properties.h>
#include <android/api-level.h>

#include <NGenXXDeviceInfo.h>

int NGenXX::Device::DeviceInfo::apiLevel()
{
    return android_get_device_api_level();
}

std::string NGenXX::Device::DeviceInfo::sysProperty(const std::string &k)
{
    char v[PROP_VALUE_MAX];
    __system_property_get(k.c_str(), v);
    return v[0] ? v : "";
}

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeAndroid;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    return sysProperty(SYS_PROPERTY_MODEL);
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    return sysProperty(SYS_PROPERTY_MANUFACTURER);
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    return sysProperty(SYS_PROPERTY_VERSION_RELEASE);
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#endif
    return NGenXXDeviceCpuArchARM;
}

#endif
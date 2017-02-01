#ifndef PATHTRACER_OPENCLPLATFORMLIST_H
#define PATHTRACER_OPENCLPLATFORMLIST_H

#include <CL/cl.hpp>

class OpenCLPlatformList {

    int platform_count;
    char** platforms_names;
    char*** platforms_devices_names;
    int* platform_device_count;

public:

    OpenCLPlatformList();

    int FindPlatformIndex(const cl::Platform& platform);
    int FindDeviceIndex(const cl::Device& device, const cl::Platform& platform);

    const char** getPlatformNames();
    int getPlatformCount();
    const char** getDevicesNames(int platform_index);
    int getDeviceCount(int platform_index);

    cl::Platform getPlatformByIndex(int platform_index);
    cl::Device getDeviceByIndex(int device_index, int platform_index);

    cl::Platform getPlatformByVendor(const std::string& vendor);
    cl::Device getDeviceByType(cl_device_type device_type, cl::Platform platform);

    void printPlatformAndDevices();

};


#endif //PATHTRACER_OPENCLPLATFORMLIST_H

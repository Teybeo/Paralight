#include "OpenCLPlatformList.h"

#include <iostream>
#include <algorithm>

using std::string;
using std::vector;
using std::cout;
using std::endl;

OpenCLPlatformList::OpenCLPlatformList() {

    vector<cl::Platform> platforms;
    // If the system had no opencl support, wouldn't the app crashed loading OpenCL.dll anyway ?
    if (cl::Platform::get(&platforms) != CL_SUCCESS) {
        cout << "- No OpenCL platforms available" << endl;
        throw std::exception();
    }

    platform_count = (int) platforms.size();

    platforms_names         = new char*  [platform_count];
    platforms_devices_names = new char** [platform_count];
    platform_device_count   = new int    [platform_count];

    // Cache the platforms names, devices names and device per platform count
    for (int p = 0; p < platform_count; ++p) {
        platforms_names[p] = new char[256];
        strncpy(platforms_names[p], platforms.at(p).getInfo<CL_PLATFORM_NAME>().c_str(), 256);

        vector<cl::Device> devices;
        platforms.at(p).getDevices(CL_DEVICE_TYPE_ALL, &devices);
        platform_device_count[p] = (int) devices.size();

        platforms_devices_names[p] = new char*[platform_device_count[p]];
        for (int d = 0; d < platform_device_count[p]; ++d) {
            platforms_devices_names[p][d] = new char[256];
            strncpy(platforms_devices_names[p][d], devices[d].getInfo<CL_DEVICE_NAME>().c_str(), 256);
        }
    }
}

int OpenCLPlatformList::FindPlatformIndex(const cl::Platform& platform) {

    const char* platform_name = platform.getInfo<CL_PLATFORM_NAME>().c_str();

    for (int p = 0; p < platform_count; ++p) {
        if (strcmp(platforms_names[p], platform_name) == 0)
            return p;
    }
    return -1;
}

int OpenCLPlatformList::FindDeviceIndex(const cl::Device& device, const cl::Platform& platform) {

    int platform_index = FindPlatformIndex(platform);
    if (platform_index == -1)
        return -1;

    // Keep the returned string alive while we use its char pointer
    const string& tmp = device.getInfo<CL_DEVICE_NAME>();
    const char* device_name = tmp.c_str();

    for (int d = 0; d < platform_device_count[platform_index]; ++d) {
        if (strcmp(platforms_devices_names[platform_index][d], device_name) == 0)
            return d;
    }
    return -1;
}

const char** OpenCLPlatformList::getPlatformNames() {
    return (const char**) platforms_names;
}

int OpenCLPlatformList::getPlatformCount() {
    return platform_count;
}

const char** OpenCLPlatformList::getDevicesNames(int platform_index) {
    return (const char**) platforms_devices_names[platform_index];
}

int OpenCLPlatformList::getDeviceCount(int platform_index) {
    return platform_device_count[platform_index];
}

cl::Platform OpenCLPlatformList::getPlatformByIndex(int platform_index) {

    const char* platform_name = platforms_names[platform_index];

    vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    cl::Platform platform = *std::find_if(platforms.begin(), platforms.end(), [=] (cl::Platform platform_i) {
//        return platform_i.getInfo<CL_PLATFORM_NAME>() == platform_name;
        return strcmp(platform_i.getInfo<CL_PLATFORM_NAME>().c_str(), platform_name) == 0;
    });

    std::cout << "found " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

    return platform;
}

cl::Device OpenCLPlatformList::getDeviceByIndex(int device_index, int platform_index) {

    const char* device_name = platforms_devices_names[platform_index][device_index];

    cl::Platform platform = getPlatformByIndex(platform_index);
    std::cout << "found " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

    vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    cl::Device device = *std::find_if(devices.begin(), devices.end(), [=] (cl::Device device_i) {
//        return device_i.getInfo<CL_DEVICE_NAME>() == device_name;
        return strcmp(device_i.getInfo<CL_DEVICE_NAME>().c_str(), device_name) == 0;
    });

    return device;
}

void OpenCLPlatformList::printPlatformAndDevices() {

    vector<cl::Platform> platforms;

    if (cl::Platform::get(&platforms) != CL_SUCCESS) {
        cout << "- No OpenCL platforms available" << endl;
        return;
    }

    for (auto platform : platforms) {
        cout << platform.getInfo<CL_PLATFORM_NAME>() << endl;
        cout << '\t' << platform.getInfo<CL_PLATFORM_VENDOR>() << endl;
        cout << '\t' << platform.getInfo<CL_PLATFORM_VERSION>() << endl;
        cout << '\t' << platform.getInfo<CL_PLATFORM_PROFILE>() << endl;
        vector<cl::Device> devices;
        if (platform.getDevices(CL_DEVICE_TYPE_ALL, &devices) != CL_SUCCESS) {
            cout << "\tNo devices available for this platform" << endl;
            continue;
        }
        for (auto device : devices) {
            cout << "\t" << device.getInfo<CL_DEVICE_NAME>() << endl;
            cout << "\t\t Driver: "             << device.getInfo<CL_DRIVER_VERSION>() << endl;
            cout << "\t\t OpenCL C: "           << device.getInfo<CL_DEVICE_OPENCL_C_VERSION>() << endl;
            cout << "\t\t Max Workgroup size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl;
            auto max_items_sizes =  device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
            cout << "\t\t Max Workitems sizes: " << max_items_sizes[0] << ", " << max_items_sizes[1] << ", " << max_items_sizes[2] << endl;
            cout << "\t\t Max Compute Units: "  << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
        }
    }

}

cl::Platform OpenCLPlatformList::getPlatformByVendor(const string& vendor) {
    // TODO: Cache the platforms vector ?
    vector<cl::Platform> platforms;
    if (cl::Platform::get(&platforms) != CL_SUCCESS) {
        cout << "- No OpenCL platforms detected";
        throw std::exception();
    }

    if (vendor == "") {
        cout << "- No platform vendor specified" << endl;
        cout << "- Fallback on the first platform available\n";
        return platforms.front();
    }

    for (auto _platform : platforms) {
        if (_platform.getInfo<CL_PLATFORM_VENDOR>().find(vendor) != string::npos) {
            return _platform;
        }
    }

    cout << "- No platforms by " << vendor << " found\n";
    cout << "- Fallback on the first platform available\n";
    return platforms.front();
}

cl::Device OpenCLPlatformList::getDeviceByType(cl_device_type device_type, cl::Platform platform) {

    vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    for (auto device : devices) {
        if (device.getInfo<CL_DEVICE_TYPE>() == device_type)
            return device;
    }

    cout << "- No OpenCL device of type " << device_type << " found\n";
    cout << "- Fallback on the first device available\n";

    return devices.front();
}


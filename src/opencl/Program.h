#ifndef OPENCL_KERNEL_H
#define OPENCL_KERNEL_H

#include "CL/cl.hpp"

class Program {

    std::vector<std::pair<std::string, unsigned int>> sources;
    std::vector<std::string> build_options;
    cl::Program::Sources cl_sources;
public:

    cl::Program prog;

    Program() = default;
    Program(std::vector<std::string> source_array, const std::vector<std::string>& build_options);

    void Build(cl::Context context, cl::Device device);

    bool Refresh(cl::Context context, cl::Device device, bool force_reload = false);

    void AddBuildOption(const char* option);

    void RemoveBuildOption(const char* option);

    void DumpBinaries(std::string filename);
};


#endif //OPENCL_KERNEL_H

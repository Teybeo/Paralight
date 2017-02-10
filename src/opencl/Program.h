#ifndef OPENCL_KERNEL_H
#define OPENCL_KERNEL_H

#include "CL/cl.hpp"
#include <set>

class Program {

    std::set<std::string> build_options;
    std::vector<std::string> sources;
    std::vector<std::pair<std::string, unsigned int>> files_timestamps;

public:

    cl::Program prog;

    Program() = default;
    Program(std::vector<std::string> source_array, const std::set<std::string>& build_options);

    void Build(cl::Context context, cl::Device device);

    bool HasChanged(bool force_reload);

    void SetBuildOption(const char* build_option, bool enabled);

    void DumpBinaries(std::string filename);
};


#endif //OPENCL_KERNEL_H

#ifndef OPENCL_KERNELCOMPILER_H
#define OPENCL_KERNELCOMPILER_H

#include <CL/cl.hpp>

class ProgramBuilder {

    const char* build_options;
    cl::Context context;
    cl::Device device;

public:
    ProgramBuilder(const char* build_options, cl::Context context, cl::Device);
    cl::Program LoadAndCompile(std::string path);
    cl::Program LinkPrograms(std::vector<cl::Program>& progs);
    static std::string LoadSource(std::string path);
    static void CheckAndPrintLog(cl::Program& prog, cl::Device device);

};

#endif //OPENCL_KERNELCOMPILER_H

#include "ProgramBuilder.h"

#include <fstream>
#include <iostream>
#include <renderers/OpenCLRenderer.h>

using std::cout;
using std::endl;
using std::string;
using std::vector;

const string CLGetBuildStatusString(int code);

ProgramBuilder::ProgramBuilder(const char* build_options, cl::Context context, cl::Device device)
        : build_options{build_options}, context{context}, device{device} {
}

cl::Program ProgramBuilder::LoadAndCompile(string path) {

//    cout << "Compiling [" << path << "] ..." << endl;

    string source = LoadSource(path);

    cl::Program prog = cl::Program{context, source};

    prog.compile(build_options);

    CheckAndPrintLog(prog, device);

    return prog;
}

cl::Program ProgramBuilder::LinkPrograms(std::vector<cl::Program>& progs) {

    cl::Program final;
    cout << "Linking programs ..." << endl;

    final = cl::linkProgram(progs);

    CheckAndPrintLog(final, device);

    return final;
}

void ProgramBuilder::CheckAndPrintLog(cl::Program& prog, cl::Device device) {

    int status = prog.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device);
    string log = prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

    // Some drivers returned small empty strings...
    if (log.length() > 3)
        cout << log;

    if (status < CL_SUCCESS) {
        cout << "Build status " << status << ": " << CLGetBuildStatusString(status) << endl;
        throw std::bad_exception();
    }
}

string ProgramBuilder::LoadSource(string path) {

    std::ifstream input(path);
    if (input.is_open() == false) {
        cout << string("Error opening [") + path + "] file" << endl;
        throw std::bad_exception();
    }

    string line;
    string source;
    while (std::getline(input, line)) {
        source.append(line).append("\n");
    }
    input.close();

    return source;
}

#define CASE_MACRO(name) case name: return #name;
const string CLGetBuildStatusString(int code) {
    switch (code) {
        CASE_MACRO(CL_BUILD_SUCCESS)
        CASE_MACRO(CL_BUILD_NONE)
        CASE_MACRO(CL_BUILD_ERROR)
        CASE_MACRO(CL_BUILD_IN_PROGRESS)
        default:
            return "Unknown build status code: " + std::to_string(code);
    }
}
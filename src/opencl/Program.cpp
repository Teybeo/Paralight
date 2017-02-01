#include "Program.h"

#include "ProgramBuilder.h"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <SDL_timer.h>

using std::string;
using std::vector;

// Path to the be searched for #include header
const string SOURCE_DIR = "../../kernel";
const string INCLUDE_ARG = "-I " + SOURCE_DIR + " ";

// Retrieves the last-write time in millisecondes of a file since 00:00
uint32_t GetLastWriteTime(const char* filename);

Program::Program(vector<string> file_array, const vector<string>& _build_options)
        : build_options{_build_options}
{
    build_options.insert(build_options.begin(), INCLUDE_ARG);

    for (const auto& build_option : build_options) {
        std::cout << build_option << std::endl;
    }

    for (string file : file_array) {
        file = SOURCE_DIR + '/' + file;
        sources.push_back(make_pair(file, GetLastWriteTime(file.c_str())));

        string* source = new string (ProgramBuilder::LoadSource(file));
        cl_sources.push_back(std::make_pair(source->c_str(), source->size()));
    }
}

void Program::Build(cl::Context context, cl::Device device) {

    string serialized_build_options = std::accumulate(build_options.begin(), build_options.end(), string());

    ProgramBuilder builder {serialized_build_options.c_str(), context, device};

    prog = cl::Program {context, cl_sources};

    prog.build(serialized_build_options.c_str());

    ProgramBuilder::CheckAndPrintLog(prog, device);

    /*vector<cl::Program> progs;

    for (auto source : sources) {
        cl::Program prog = builder.LoadAndCompile(source.first);
        progs.emplace_back(prog);
    }

    prog = builder.LinkPrograms(progs);*/
}


bool Program::Refresh(cl::Context context, cl::Device device, bool force_reload) {

    static uint32_t lastCheck = 0;

    // If current time is within 100ms of last check, do nothing
    if (SDL_GetTicks() < lastCheck + 100)
        return false;

    lastCheck = SDL_GetTicks();

    for (std::pair<string, unsigned int>& source : sources) {

        uint32_t new_timestamp = GetLastWriteTime(source.first.c_str());
        if (new_timestamp != source.second || force_reload) {
            try {
//                printf("old time: %ud, new time: %ud\n", source.second, new_timestamp);
                source.second = new_timestamp;
                Build(context, device);
            } catch (const std::bad_exception& e) {
                std::cout << "Exeption came from kernel refresh" << std::endl;
                return false;
            }
            return true;
        }
    }

    return false;
}

void Program::AddBuildOption(const char* option) {

    auto it = std::find(build_options.begin(), build_options.end(), option);

    if (it == build_options.end()) {
        build_options.emplace_back(option);
    }
}

void Program::RemoveBuildOption(const char* option) {

    auto it = std::find(build_options.begin(), build_options.end(), option);

    if (it != build_options.end()) {
        build_options.erase(it);
    }
}

/// Retrieves the last-write time in millisecondes of a file since 00:00
uint32_t GetLastWriteTime(const char* filename)
{
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME stUTC, stLocal;
    unsigned int writeTime = 0;

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    // Retrieve the file times for the file.
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
        puts("Error GetFiletime");

    // Convert the last-write time to local time.
    FileTimeToSystemTime(&ftWrite, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Convert and add the separate units in milliseconds
    // 23:59:59 = 84 399 000, donc ca tient sur un uint
    writeTime = (stLocal.wHour * 3600 + stLocal.wMinute * 60 + stLocal.wSecond) * 1000U;

    CloseHandle(hFile);

    return writeTime;
}

void Program::DumpBinaries(string filename) {

    cl_uint program_num_devices;

    clGetProgramInfo(prog(), CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &program_num_devices, NULL);

    size_t binaries_sizes[program_num_devices];

    clGetProgramInfo(prog(), CL_PROGRAM_BINARY_SIZES, program_num_devices*sizeof(size_t), binaries_sizes, NULL);

    char** binaries = new char*[program_num_devices];

    for (size_t i = 0; i < program_num_devices; i++)
        binaries[i] = new char[binaries_sizes[i] + 1];

    clGetProgramInfo(prog(), CL_PROGRAM_BINARIES, program_num_devices*sizeof(size_t), binaries, NULL);

    for (size_t i = 0; i < program_num_devices; i++)
    {
        binaries[i][binaries_sizes[i]] = '\0';
    }

    std::ofstream out_binary_file {filename + ".ptx"};

    for (size_t i = 0; i < binaries_sizes[0]; i++)
        out_binary_file << binaries[0][i];

    out_binary_file.close();
}

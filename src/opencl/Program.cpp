#include "Program.h"

#include "ProgramBuilder.h"

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
#elif __APPLE__
	#include <sys/stat.h>
#endif

#include <algorithm>
#include <iostream>
#include <fstream>
#include <SDL_timer.h>
#include <memory>
#include <numeric>

using std::string;
using std::vector;
using std::set;
using std::shared_ptr;

// Path to the be searched for #include header
const string SOURCE_DIR = "../../kernel";
const string INCLUDE_ARG = "-I " + SOURCE_DIR + " ";

// Retrieves the last-write time in millisecondes of a file since 00:00
uint32_t GetLastWriteTime(const char* filename);

Program::Program(vector<string> file_array, const set<string>& _build_options)
        : build_options{_build_options}
{
    build_options.insert(INCLUDE_ARG);

    for (const auto& build_option : build_options) {
        std::cout << build_option << std::endl;
    }

    for (string file : file_array) {
        file = SOURCE_DIR + '/' + file;
        files_timestamps.push_back(make_pair(file, GetLastWriteTime(file.c_str())));
    }
}

void Program::Build(cl::Context context, cl::Device device) {

	string serialized_build_options;
	for (const auto& build_option : build_options)
		serialized_build_options.append(build_option + " ");

    ProgramBuilder builder {serialized_build_options.c_str(), context, device};

    sources.clear();
    cl::Program::Sources cl_sources;

    for (const auto& file : files_timestamps) {
        sources.push_back(ProgramBuilder::LoadSource(file.first));
        cl_sources.push_back(std::make_pair(sources.back().c_str(), sources.back().size()));
    }

    prog = cl::Program {context, cl_sources};

    prog.build(serialized_build_options.c_str());

    ProgramBuilder::CheckAndPrintLog(prog, device);

    /*vector<cl::Program> progs;

    for (auto file : files_timestamps) {
        cl::Program prog = builder.LoadAndCompile(file.first);
        progs.emplace_back(prog);
    }

    prog = builder.LinkPrograms(progs);*/
}

bool Program::HasChanged(bool force_reload) {

    static uint32_t lastCheck = 0;

    // If current time is within 100ms of last check, do nothing
    if (SDL_GetTicks() < lastCheck + 100)
        return false;

    lastCheck = SDL_GetTicks();

    for (std::pair<string, unsigned int>& file : files_timestamps) {

        uint32_t current_timestamp = GetLastWriteTime(file.first.c_str());

        if (current_timestamp != file.second || force_reload) {
//            printf("old time: %ud, new time: %ud\n", file.second, current_timestamp);
            file.second = current_timestamp;
            return true;
        }
    }

    return false;
}

void Program::SetBuildOption(const char* build_option, bool enabled) {

    if (enabled) {
        build_options.emplace(build_option);
    } else {
        build_options.erase(build_option);
    }
}
/// Retrieves the last-write time in millisecondes of a file since 00:00
uint32_t GetLastWriteTime(const char* filename)
{
#ifdef WIN32
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
#elif __APPLE__
	struct stat s = {};
	stat(filename, &s);
	uint32_t time = static_cast<uint32_t>(s.st_mtimespec.tv_sec);
	return time;
#else
	return 0;
#endif
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


#ifndef PARALIGHT_CHRONOMETER_H
#define PARALIGHT_CHRONOMETER_H

#include <chrono>

class Chronometer {
    // On MinGW-w64, steady_clock is more precise than high_resolution_clock
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63400
    typedef std::chrono::steady_clock clock;
    
    clock::time_point start = clock::now();

public:
    
    Chronometer() = default;

    float GetMilliseconds() const {
        auto end = clock::now();
        return std::chrono::duration<float, std::milli>(end - start).count();
    }
    
    float GetSeconds() const {
        auto end = clock::now();
        return std::chrono::duration<float>(end - start).count();
    }
    
    void Restart() {
        start = clock::now();
    }
    
};


#endif //PARALIGHT_CHRONOMETER_H

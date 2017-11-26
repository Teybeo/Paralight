#ifndef PARALIGHT_CHRONOMETER_H
#define PARALIGHT_CHRONOMETER_H

#include <chrono>

class Chronometer {
    
    typedef std::chrono::high_resolution_clock clock;
    
    clock::time_point start = clock::now();

public:
    Chronometer() {}
    
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

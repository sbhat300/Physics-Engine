#ifndef SETUP_H
#define SETUP_H

#include <atomic>

namespace setup
{
    extern int maxLayers;
    extern float fixedDeltaTime;
    extern float linearDamping;
    extern float angularDamping;
    extern bool windows;
    extern std::atomic<bool> shouldWindowClose;
};

#endif
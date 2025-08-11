#pragma once

#include <thread>

#include <affinity/cpuinfo/cpu_info.hpp>
#include <core/logger/logger.hpp>

namespace fastshogi::affinity::cpu_info {

// Some dumb code for macOS, setting the affinity is not really supported.
inline CpuInfo getCpuInfo() noexcept {
    LOG_TRACE("Getting CPU info");

    CpuInfo cpu_info;

    for (int i = 0; i < static_cast<int>(std::thread::hardware_concurrency()); ++i) {
        CpuInfo::PhysicalCpu::Core core = {i, {i}};

        cpu_info.physical_cpus[0].cores[i] = core;
    }

    return cpu_info;
}

}  // namespace fastshogi::affinity::cpu_info

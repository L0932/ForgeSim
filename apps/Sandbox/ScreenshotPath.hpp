#pragma once
#include "ScreenshotRequest.hpp"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace ForgeSim::Sandbox
{
    [[nodiscard]] inline std::filesystem::path NextScreenshotPath(const std::filesystem::path& directory,
        ScreenshotTarget target,std::chrono::system_clock::time_point now=std::chrono::system_clock::now())
    {
        using namespace std::chrono;
        const auto day=floor<days>(now);const year_month_day date{day};const hh_mm_ss time{floor<milliseconds>(now)-day};
        std::ostringstream stem;
        stem<<"ForgeSim-"<<(target==ScreenshotTarget::Scene?"Scene-":"Editor-")<<std::setfill('0')
            <<std::setw(4)<<static_cast<int>(date.year())<<std::setw(2)<<static_cast<unsigned>(date.month())
            <<std::setw(2)<<static_cast<unsigned>(date.day())<<'T'<<std::setw(2)<<time.hours().count()
            <<std::setw(2)<<time.minutes().count()<<std::setw(2)<<time.seconds().count()
            <<'-'<<std::setw(3)<<time.subseconds().count()<<'Z';
        for(unsigned sequence=0;sequence<100000;++sequence)
        {
            const auto file=directory/(stem.str()+(sequence?"-"+std::to_string(sequence):"")+".png");
            if(!std::filesystem::exists(file)) return file;
        }
        throw std::runtime_error("Cannot allocate a unique screenshot filename.");
    }
}

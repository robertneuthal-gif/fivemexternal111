#pragma once

#include <iostream>
#include <ctime>
#include <cstdarg>
#include <array>
#include <string>
#include <sstream>





class upx_utility {
public:
    enum class log_level {
        INFO,
        WARNING,
        SUCCESS,
        FAIL
    };

private:
    static constexpr const char* RESET = "\033[0m";
    static constexpr const char* WHITE = "\033[97m";
    static constexpr const char* BLUE = "\033[94m";
    static constexpr const char* YELLOW = "\033[93m";
    static constexpr const char* GREEN = "\033[92m";
    static constexpr const char* RED = "\033[91m";

    static const char* get_color(log_level level) {
        switch (level) {
        case log_level::INFO:    return BLUE;
        case log_level::WARNING: return YELLOW;
        case log_level::SUCCESS: return GREEN;
        case log_level::FAIL:   return RED;
        default:                 return RESET;
        }
    }

    static const char* get_level_string(log_level level) {
        switch (level) {
        case log_level::INFO:    return "INFO";
        case log_level::WARNING: return "WARNING";
        case log_level::SUCCESS: return "SUCCESS";
        case log_level::FAIL:   return "ERROR";
        default:                 return "UNKNOWN";
        }
    }

    static std::string current_time() {
        std::array<char, 80> buffer{};
        std::time_t rawtime;
        std::tm timeinfo;

        std::time(&rawtime);
#ifdef _WIN32
        localtime_s(&timeinfo, &rawtime);
#else
        localtime_r(&rawtime, &timeinfo);
#endif
        std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return std::string(buffer.data());
    }

public:
    static void s_print(log_level level, const char* meta, const char* format, ...) {
        const char* color = get_color(level);
        const char* level_str = get_level_string(level);
        std::string time_str = current_time();

        std::ostringstream header;
        header << WHITE << "[" << color << level_str << WHITE << "] ";
        header << WHITE << "[" << time_str << "] ";

        if (meta && std::strlen(meta) > 0) {
            header << WHITE << "[" << GREEN << meta << WHITE << "] ";
        }

        std::cout << header.str();

        va_list args;
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);

        std::cout << RESET << std::endl;
    }

};

inline upx_utility utils;

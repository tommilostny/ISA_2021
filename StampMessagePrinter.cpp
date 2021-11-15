/**
 * @brief Message printer class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <chrono>
#include <iomanip>
#include <iostream>
#include "StampMessagePrinter.hpp"

void StampMessagePrinter::Print(std::string message)
{
    PrintWithTimeStamp(message, std::cout);
}

void StampMessagePrinter::PrintError(std::string message)
{
    PrintWithTimeStamp(message, std::cerr);
}

void StampMessagePrinter::PrintWithTimeStamp(std::string message, std::ostream& stream)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    //[YYYY-MM-DD hh:mm:ss.uu] message
    stream << "[" 
        << std::put_time(std::localtime(&time), "%F %T") << "." << std::setfill('0') << std::setw(3)
        << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000
        << "] " 
        << message << std::endl;
}

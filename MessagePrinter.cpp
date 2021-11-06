/**
 * @brief Message printer class implementation.
 * @author Tomáš Milostný (xmilos02)
 */
#include <chrono>
#include <iomanip>
#include <iostream>
#include "MessagePrinter.hpp"

void MessagePrinter::PrintMessage(std::string message)
{
    PrintWithTimeStamp(message, std::cout);
}

void MessagePrinter::PrintError(std::string message)
{
    PrintWithTimeStamp(message, std::cerr);
}

void MessagePrinter::PrintWithTimeStamp(std::string message, std::ostream& stream)
{
    stream << "[" << GetTimeStamp() << "] " << message << std::endl;
}

std::string MessagePrinter::GetTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%F %T") << "." << std::setfill('0') << std::setw(3)
        << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    return ss.str();
}

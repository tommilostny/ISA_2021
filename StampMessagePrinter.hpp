/**
 * @brief Message printer class module.
 * @author Tomáš Milostný (xmilos02)
 */
#pragma once
#include <string>

class StampMessagePrinter
{
public:
    static void Print(std::string message);
    static void PrintError(std::string message);
private:
    StampMessagePrinter();
    //Print local time with miliseconds and message.
    static void PrintWithTimeStamp(std::string message, std::ostream& stream);
};

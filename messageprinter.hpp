#include <string>

class MessagePrinter
{
public:
    static void PrintMessage(std::string message);
    static void PrintError(std::string message);
private:
    MessagePrinter();
    //Print local time with miliseconds and message.
    static void PrintWithTimeStamp(std::string message, std::ostream& stream);
    static std::string GetTimeStamp();
};

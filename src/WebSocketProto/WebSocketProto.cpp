#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS 1

#include <vector>
#include <Windows.h>
#include "WinHttp.hpp"
#include "testmessage.pb.h"

#pragma comment(lib, "winhttp.lib")

int main(int argc, char** args)
{
    TestMessage ts;
    ts.set_sometext("poop");

    WinHttp::WinHttpWebSocket ws(L"https://127.0.0.1", 51935, true);
    ws.Connect();
    //ws.Receive(receivedBuffer);

    std::vector<char> buffer;
    buffer.resize(256);
    if (ts.SerializeToArray(&buffer[0], buffer.size()))
        std::cout << "OK" << std::endl;
    //std::string msg(buffer.begin(), buffer.end());
    //buffer.shrink_to_fit();

    //ws.SendString("Hoohee");
    ws.SendBuffer(buffer);

    std::string receivedBuffer;
    ws.Receive(receivedBuffer);
    TestMessage ts2;
    ts2.ParseFromArray(&receivedBuffer[0], receivedBuffer.size());
    std::cout << ts2.sometext() << std::endl;

    return 0;
}

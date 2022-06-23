
#include <iostream>

int main(int argc, char* argv[])
{
    return system("powershell -ExecutionPolicy Bypass -F test.ps1 > script.log");
}

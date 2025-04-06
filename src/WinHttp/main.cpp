#pragma comment(lib, "winhttp.lib")

import std;
import winhttpclient;

int main()
{
    Client::WinHttpClient client(L"google.com");
    std::println("{}", client.Get(L"").ResponseBody);
    return 0;
}


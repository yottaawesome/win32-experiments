#pragma comment(lib, "winhttp.lib")

import std;
import winhttpclient;

int main()
{
    new auto([d = std::array<int, 1024>()]() {return d[0]; });

    Client::WinHttpClient client(L"google.com");
    std::println("{}", client.Get(L"").ResponseBody);
    return 0;
}


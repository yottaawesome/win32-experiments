import pipelib;
import stdlib;

auto main(int argc, char* argv[]) noexcept -> int
try
{
    if (argc != 2)
        return 1;

    std::string pipeClientHandle(argv[1]);
    if (pipeClientHandle.empty())
        return 2;

    Win32::HANDLE hClientPipe = reinterpret_cast<Win32::HANDLE>(std::strtoull(pipeClientHandle.c_str(), nullptr, 16));
    PipeLib::UniqueHandle clientPipe{ hClientPipe };

    std::pair readOperation = PipeLib::ReadWithHeader(clientPipe.get());
    std::string readString{ reinterpret_cast<char*>(readOperation.second.data()), readOperation.second.size() };
    std::println(
R"(Read message:
 -> Header type: {}
 -> Subtype: {}
 -> Data: {})",
        static_cast<int>(readOperation.first.Type),
        static_cast<int>(readOperation.first.Subtype),
        readString
    );

    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
    return 1;
}
catch (...)
{
    std::println("Unknown exception");
    return 1;
}

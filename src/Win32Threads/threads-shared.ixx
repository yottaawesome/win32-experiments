export module threads:shared;
import std;
import win32;

export
{
    struct HandleDeleter { void operator()(win32::HANDLE h) { win32::CloseHandle(h); } };
    using HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<win32::HANDLE>, HandleDeleter>;

    struct system_category_error : public std::system_error
    {
        system_category_error(std::string_view msg, win32::DWORD errorCode = win32::GetLastError())
            : system_error(std::error_code{ static_cast<int>(errorCode), std::system_category() }, std::string{ msg })
        {}
    };
}

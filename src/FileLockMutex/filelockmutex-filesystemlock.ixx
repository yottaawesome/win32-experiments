export module filelockmutex:filesystemlock;
import std;
import :win32;

export namespace Strings
{
	template<size_t N>
	struct FixedStringW
	{
		wchar_t data[N]{};
		constexpr FixedStringW(const wchar_t(&str)[N]) noexcept
		{
			std::copy_n(str, N, data);
		}
		constexpr auto ToView(this auto&& self) noexcept -> std::wstring_view
		{
			return std::wstring_view(self.data, N - 1); // exclude null terminator
		}
	};
}

export namespace Raii
{
	template<auto VDeleteFn>
	struct Deleter
	{
		constexpr static void operator()(auto handle) noexcept
		{
			VDeleteFn(handle);
		}
	};
	template<typename T, auto VDeleteFn>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleteFn>>;

	using HandleUniquePtr = IndirectUniquePtr<Win32::HANDLE, Win32::CloseHandle>;
}

export namespace Error
{
	struct Win32Error : std::runtime_error
	{
		Win32Error(Win32::DWORD errorCode, std::string_view message)
			: std::runtime_error(std::format("{} (error code {:#x})", message, errorCode))
			, code(errorCode)
		{}

		constexpr auto Code() const noexcept -> Win32::DWORD
		{
			return code;
		}
	private:
		Win32::DWORD code;
	};
}

export namespace Process
{
	struct StartupInfo : Win32::STARTUPINFOW
	{
		constexpr StartupInfo() noexcept
			: Win32::STARTUPINFOW{ .cb = sizeof(Win32::STARTUPINFOW) }
		{ }
	};

	struct ProcessInformation : Win32::PROCESS_INFORMATION
	{
		~ProcessInformation()
		{
			Close();
		}

		constexpr ProcessInformation() noexcept
			: Win32::PROCESS_INFORMATION{}
		{ }

		ProcessInformation(const ProcessInformation&) = delete;
		auto operator=(const ProcessInformation&) -> ProcessInformation& = delete;

		ProcessInformation(ProcessInformation&& other) noexcept
		{
			Move(other);
		}

		auto operator=(ProcessInformation&& other) noexcept -> ProcessInformation&
		{
			Move(other);
			return *this;
		}

		void Close() noexcept
		{
			if (hProcess)
			{
				Win32::CloseHandle(hProcess);
				hProcess = nullptr;
			}
			if (hThread)
			{
				Win32::CloseHandle(hThread);
				hThread = nullptr;
			}
		}

		void Move(ProcessInformation& other) noexcept
		{
			Close();
			hProcess = other.hProcess;
			hThread = other.hThread;
			dwProcessId = other.dwProcessId;
			dwThreadId = other.dwThreadId;
			other.hProcess = nullptr;
			other.hThread = nullptr;
			other.dwProcessId = 0;
			other.dwThreadId = 0;
		}
	};

	auto GetCurrentExecutablePath() -> std::filesystem::path
	{
		constexpr size_t blockSize = 2048;
		std::wstring filePath(L"\0", 0);
		Win32::DWORD status = Win32::ErrorCodes::InsufficientBuffer;
		Win32::DWORD count = 0;
		while (status == Win32::ErrorCodes::InsufficientBuffer)
		{
			filePath.resize(filePath.size() + blockSize);
			count = Win32::GetModuleFileNameW(nullptr, &filePath[0], static_cast<Win32::DWORD>(filePath.size()));
			if (not count)
				throw Error::Win32Error(Win32::GetLastError(), "GetModuleFileNameW() failed");
			status = Win32::GetLastError();
		}
		filePath.resize(count);
		return filePath;
	}

	auto GetCurrentExecutableDirectory() -> std::filesystem::path
	{
		return GetCurrentExecutablePath().parent_path();
	}
}

export namespace Async
{
	template<Strings::FixedStringW VPath = L"FileLock.lock">
	struct FileSystemLock final
	{
		constexpr FileSystemLock() = default;

		FileSystemLock(bool acquire)
		{
			acquire ? lock() : void();
		}

		FileSystemLock(const FileSystemLock&) = delete;
		auto operator=(const FileSystemLock&) -> FileSystemLock & = delete;

		FileSystemLock(FileSystemLock&&) = default;
		auto operator=(FileSystemLock&&) -> FileSystemLock & = default;

		void lock(this FileSystemLock& self)
		{
			// Already owned
			if (self.fileHandle)
				return;

			auto handle = Win32::CreateFileW(
				VPath.ToView().data(),
				Win32::GenericRead,
				static_cast<Win32::DWORD>(Win32::FileShareMode::None),
				nullptr,
				static_cast<Win32::DWORD>(Win32::CreateFileDisposition::CreateNew),
				Win32::FileAttributes::Normal | Win32::FileFlags::DeleteOnClose, // can also use hidden
				nullptr
			);
			if (not handle or handle == Win32::InvalidHandleValue)
			{
				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, "Failed to create or open file handle.");
			}
			self.fileHandle = Raii::HandleUniquePtr(handle);
		}

		void unlock(this FileSystemLock& self) noexcept
		{
			if (not self.fileHandle)
				return;
			self.fileHandle.reset();
		}

		auto try_lock(this FileSystemLock& self) noexcept -> bool
		try
		{
			return (self.lock(), true);
		}
		catch (...)
		{
			return false;
		}

		constexpr auto IsLocked(this const FileSystemLock& self) noexcept -> bool
		{
			return self.fileHandle != nullptr;
		}

		constexpr operator bool(this const FileSystemLock& self) noexcept
		{
			return self.IsLocked();
			}

	private:
		auto Move(this FileSystemLock& self, FileSystemLock& other) -> FileSystemLock&
		{
			self.fileHandle = std::move(other.fileHandle);
			return self;
		}
		Raii::HandleUniquePtr fileHandle;
	};
}
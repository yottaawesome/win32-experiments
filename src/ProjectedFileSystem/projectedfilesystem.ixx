export module projectedfilesystem;
import common;

namespace projected_file_system
{
	struct instance_file_disposition
	{
		Util::HandleDeleter file_ptr;
		bool is_new_file = false;

		instance_file_disposition(Util::HandleDeleter&& file_ptr, bool is_new_file) noexcept
			: file_ptr(std::move(file_ptr)), is_new_file(is_new_file)
		{ }
	};
}

export namespace projected_file_system
{
	class pfs_context
	{
		public:
		pfs_context(std::filesystem::path root) : m_root(std::move(root)) 
		{
			init();
		}

		private:
		void init()
		{
			check_and_create_root();
			instance_file_disposition disp = create_or_open_instance_file();
			read_or_write_guid(disp);
		}

		bool check_and_create_root()
		{
			if (std::filesystem::is_directory(m_root))
				return true;

			if (std::filesystem::exists(m_root))
				throw std::runtime_error("Specified root is not a directory.");

			std::filesystem::create_directory(m_root);
			return false;
		}

		instance_file_disposition create_or_open_instance_file()
		{
			Win32::HANDLE hFile = nullptr;
			if (std::filesystem::exists(m_instanceFile))
			{
				hFile = Win32::CreateFileW(
					m_instanceFile.wstring().data(),
					Win32::Permission::GenericRead,
					0,
					nullptr,
					Win32::OpenExisting,
					0,
					nullptr
				);
				if (hFile == Win32::InvalidHandleValue)
					throw Error::Win32Error(Win32::GetLastError(), "Failed opening existing instance file.");
				return { Util::HandleDeleter{hFile}, false };
			}

			hFile = Win32::CreateFileW(
				m_instanceFile.wstring().data(),
				Win32::Permission::GenericWrite,
				0,
				nullptr,
				Win32::CreateNew,
				Win32::FileAttribute::Hidden,
				nullptr
			);
			if (hFile == Win32::InvalidHandleValue)
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating instance file.");
			return { Util::HandleDeleter{hFile}, true };
		}

		void read_or_write_guid(instance_file_disposition& disp)
		{
			Win32::DWORD operationBytes = 0;
			if (disp.is_new_file)
			{
				m_guid = Util::GloballyUniqueID();
				bool success = Win32::WriteFile(disp.file_ptr.get(), &m_guid.m_guid, sizeof(m_guid.m_guid), &operationBytes, nullptr);
				if (not success)
					throw Error::Win32Error(GetLastError(), "Failed writing out new GUID.");
			}
			else
			{
				bool success = Win32::ReadFile(disp.file_ptr.get(), &m_guid.m_guid, sizeof(m_guid.m_guid), &operationBytes, nullptr);
				if (not success)
					throw Error::Win32Error(GetLastError(), "Failed reading existing GUID.");
			}
		}

		private:
		std::filesystem::path m_root;
		std::filesystem::path m_instanceFile = std::format("{}\\objproj.guid", m_root.string());
		Util::GloballyUniqueID m_guid{ Util::GloballyUniqueID::Null };
	};
}
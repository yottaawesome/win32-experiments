export module projectedfilesystem;
import common;

namespace
{

}

export namespace projected_file_system
{
	class pfs_context
	{
		std::filesystem::path m_root;
		std::filesystem::path m_instanceFile = std::format("{}\\objproj.guid", m_root.string());
		Util::GloballyUniqueID m_guid{ Util::GloballyUniqueID::Null };

		public:
		pfs_context(std::filesystem::path root) : m_root(std::move(root)) 
		{
			init();
		}

		private:
		struct disposition
		{
			Util::HandleDeleter File;
			bool IsNewFile = false;
		};

		void init()
		{
			check_and_create_root();
			disposition disp = create_or_open_instance_file();
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

		disposition create_or_open_instance_file()
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
					Win32::FileAttribute::Hidden,
					nullptr
				);
				if (hFile == Win32::InvalidHandleValue)
					throw Error::Win32Error(Win32::GetLastError(), "Failed opening existing instance file.");
				return disposition{ Util::HandleDeleter{hFile}, false };
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
			return disposition{ Util::HandleDeleter{hFile}, true };
		}

		void read_or_write_guid(disposition& disp)
		{
			if (disp.IsNewFile)
				m_guid = Util::GloballyUniqueID();
		}
	};
}
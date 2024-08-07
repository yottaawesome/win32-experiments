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
		void init()
		{
			check_and_create_root();
			create_or_open_instance_file();
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

		void create_or_open_instance_file()
		{
			Util::HandleDeleter file;
			if (std::filesystem::exists(m_instanceFile))
			{
				
			}
			else
			{
				Win32::HANDLE hFile = Win32::CreateFileW(
					m_instanceFile.wstring().data(),
					Win32::Permission::GenericWrite,
					0,
					nullptr,
					Win32::CreateNew,
					Win32::FileAttribute::Hidden,
					nullptr
				);
				file = Util::HandleDeleter(hFile);
				m_guid = Util::GloballyUniqueID();
			}
		}
	};
}
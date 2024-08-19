export module projectedfilesystem;
import common;

namespace projected_file_system
{
	struct instance_file_disposition
	{
		RAII::HandleDeleter file_ptr;
		bool is_new_file = false;

		instance_file_disposition(RAII::HandleDeleter&& file_ptr, bool is_new_file) noexcept
			: file_ptr(std::move(file_ptr)), is_new_file(is_new_file)
		{ }
	};

	namespace callbacks
	{
		template<typename T>
		Win32::HRESULT start_directory_enumeration(const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData, const Win32::GUID* enumerationId)
		{
			return reinterpret_cast<T*>(callbackData->InstanceContext)->start_directory_enumeration(callbackData, enumerationId);
		}

		template<typename T>
		Win32::HRESULT get_directory_enumeration(
			const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData,
			const Win32::GUID* enumerationId,
			Win32::PCWSTR searchExpression,
			Win32::ProjectedFileSystem::PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle
		)
		{
			return reinterpret_cast<T*>(callbackData->InstanceContext)->get_directory_enumeration(callbackData, enumerationId, searchExpression, dirEntryBufferHandle);
		}

		template<typename T>
		Win32::HRESULT get_placeholder_information(const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData)
		{
			return reinterpret_cast<T*>(callbackData->InstanceContext)->get_placeholder_information(callbackData);
		}

		template<typename T>
		Win32::HRESULT get_file_data(
			const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData,
			std::uint64_t byteOffset,
			std::uint32_t length
		)
		{
			return reinterpret_cast<T*>(callbackData->InstanceContext)->get_file_data(callbackData, byteOffset, length);
		}

		template<typename T>
		Win32::HRESULT end_directory_enumeration(
			const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData,
			const Win32::GUID* enumerationId
		)
		{
			return reinterpret_cast<T*>(callbackData->InstanceContext)->end_directory_enumeration(callbackData, enumerationId);
		}
	}
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
			start_virtualising();
		}

		void start_virtualising()
		{
			Win32::HRESULT hr = Win32::ProjectedFileSystem::PrjMarkDirectoryAsPlaceholder(
				m_root.c_str(), 
				nullptr, 
				nullptr, 
				&m_guid.m_guid
			);
			Error::CheckHResult(hr, "PrjMarkDirectoryAsPlaceholder() failed.");
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
			bool exists = std::filesystem::exists(m_instanceFile);
			if (exists)
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
			}
			else
			{
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
			}
			
			return { RAII::HandleDeleter{hFile}, not exists };
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

		Win32::HRESULT start_directory_enumeration(const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData, const GUID* enumerationId)
		{
			return Win32::HrCodes::OK;
		}

		Win32::HRESULT get_directory_enumeration(
			const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData,
			const Win32::GUID* enumerationId,
			Win32::PCWSTR searchExpression,
			Win32::ProjectedFileSystem::PRJ_DIR_ENTRY_BUFFER_HANDLE dirEntryBufferHandle
		)
		{
			return Win32::HrCodes::OK;
		}

		Win32::HRESULT get_placeholder_information(const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData)
		{
			return Win32::HrCodes::OK;
		}

		Win32::HRESULT get_file_data(const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData, std::uint64_t byteOffset, std::uint32_t length)
		{
			return Win32::HrCodes::OK;
		}

		Win32::HRESULT end_directory_enumeration(const Win32::ProjectedFileSystem::PRJ_CALLBACK_DATA* callbackData, const GUID* enumerationId)
		{
			return Win32::HrCodes::OK;
		}

		private:
		std::filesystem::path m_root;
		std::filesystem::path m_instanceFile = std::format("{}\\objproj.guid", m_root.string());
		Util::GloballyUniqueID m_guid{ Util::GloballyUniqueID::Null };
	};
}
import common;

auto main() -> int
{
	Win32::HRESULT hr = Win32::CoInitializeEx(
		nullptr, 
		Win32::COINIT::COINIT_MULTITHREADED | Win32::COINIT::COINIT_DISABLE_OLE1DDE
	);
	Error::CheckHResult(hr, "Failed initialising COM.");

	Util::ComPtr<Win32::Wuapi::IUpdateSession> session;
	hr = Win32::CoCreateInstance(
		Win32::Wuapi::CLSID_UpdateSession, 
		nullptr, 
		Win32::CLSCTX::CLSCTX_INPROC_SERVER, 
		__uuidof(session.Get()), 
		reinterpret_cast<void**>(session.ReleaseAndGetAddress())
	);
	Error::CheckHResult(hr, "Failed creating IUpdateSession.");

	Util::ComPtr<Win32::Wuapi::IUpdateSearcher> searcher;
	hr = session->CreateUpdateSearcher(searcher.ReleaseAndGetAddress());
	Error::CheckHResult(hr, "Failed creating searcher.");

	long count = 0;
	hr = searcher->GetTotalHistoryCount(&count);
	Error::CheckHResult(hr, "GetTotalHistoryCount() failed.");
	std::println("Count of total history: {}.", count);
	
	Util::ComPtr<Win32::Wuapi::IUpdateHistoryEntryCollection> history;
	long recordsToRetrieve = 100;
	hr = searcher->QueryHistory(0, recordsToRetrieve, history.ReleaseAndGetAddress());
	Error::CheckHResult(hr, "Failed querying history.");

	for (int i = 0; i < recordsToRetrieve; i++)
	{
		Util::ComPtr<Win32::Wuapi::IUpdateHistoryEntry> entry;
		hr = history->get_Item(i, entry.ReleaseAndGetAddress());
		Error::CheckHResult(hr, "Failed getting item.");

		Win32::_bstr_t title;
		//entry.Do(&Win32::Wuapi::IUpdateHistoryEntry::get_Title, title.GetAddress());
		Error::CheckHResult(entry->get_Title(title.GetAddress()));
		std::println("Title [{}]: {}", i, static_cast<char*>(title));
	}

	return 0;
}
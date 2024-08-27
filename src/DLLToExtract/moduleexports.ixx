export module moduleexports;

export extern "C" 
{
	auto __declspec(dllexport) GetTheOtherSecretOfTheUniverse() -> int
	{
		return 43;
	}
}

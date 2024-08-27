import selfextractingexe;
import std;

// Based on
// https://github.com/GiovanniDicanio/CppWinDllResourceExtractor
// https://giodicanio.com/2024/03/25/embedding-and-extracting-binary-files-like-dlls-into-an-exe-as-resources/
int main() 
try
{
	SelfExtractingExe::ExtractDLL();
	return 0;
}
catch (const std::exception& ex)
{
	std::println("Exception: {}", ex.what());
}
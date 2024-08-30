export module testdll;
import std;

export namespace TestDLL
{
	constexpr int __declspec(dllexport) DummyValue = 0;

	struct TestClass
	{
		void AA() {}
		void BB() { throw std::runtime_error("Test exception"); }
		std::string GetAString() { return "blah blah"; }
	};
}
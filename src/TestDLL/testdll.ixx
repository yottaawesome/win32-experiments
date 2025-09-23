export module testdll;
import std;

export namespace TestDLL
{
	constexpr int __declspec(dllexport) DummyValue = 0;

	__declspec(dllexport)
	void AnotherFunction() 
	{

	}

	__declspec(dllexport)
	void YetAnotherFunction();

	template<typename T>
	T ReturnIt(T t)
	{
		return t;
	}
	
	template<> __declspec(dllexport)
	float ReturnIt<float>(float f)
	{
		return f;
	}

	struct __declspec(dllexport) TestClass
	{
		void AA() {}
		void BB() { throw std::runtime_error("Test exception"); }
		std::string GetAString() { return "blah blah"; }
	};

	struct __declspec(dllexport) TestClass2
	{
		void CC();
	};
}
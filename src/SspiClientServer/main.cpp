struct A
{
	constexpr auto operator()() -> int
	{
		return 42;
	}
};

int main()
{
	auto a = new auto([a = A{}] {});
	(*a)();
	delete a;
	return 0;
}
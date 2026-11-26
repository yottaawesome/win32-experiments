export module stdx;
export import std;

export namespace stdx
{
	template<typename...Ts>
	struct Overload : public Ts...
	{
		using Ts::operator()...;
	};

	//
	// A variant wrapper that provides a convenient Visit method for invoking a visitor on the variant.
	template<typename...Ts>
	struct Variant
	{
		//
		// Fields
		std::variant<Ts...> Value;

		//
		// Operators
		constexpr auto operator()(this auto&& self, auto&&...fns) -> decltype(auto)
		{
			return self.Visit(std::forward<decltype(fns)>(fns)...);
		}

		constexpr auto operator=(this auto&& self, auto&& value) -> decltype(auto)
		{
			self.Value = std::forward_like<decltype(self)>(value);
			return self;
		}

		//
		// Methods
		constexpr auto Visit(this auto&& self, auto&&...fns) -> decltype(auto)
		{
			return std::visit(
				Overload{ std::forward<decltype(fns)>(fns)... },
				std::forward_like<decltype(self)>(self.Value));
		}

		constexpr auto VisitSome(this auto&& self, auto&&...fn) -> decltype(auto)
		{
			return std::visit(
				Overload{ std::forward<decltype(fn)>(fn)..., [](auto&&) {} },
				std::forward_like<decltype(self)>(self.Value));
		}

		consteval auto Size() const noexcept -> std::size_t
		{
			return sizeof...(Ts);
		}

		template<typename T>
		constexpr auto Emplace(this auto&& self, auto&&...args) -> decltype(auto)
		{
			self.Value.emplace<T>(std::forward<decltype(args)>(args)...);
			return self;
		}

		template<typename T>
		constexpr auto Get(this auto&& self) -> decltype(auto)
		{
			return std::get<T>(std::forward_like<decltype(self)>(self.Value));
		}
	};
}

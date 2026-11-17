export module CpuId:stdx;
import std;

export namespace stdx
{
	template<typename T, typename U>
	using ForwardLike = decltype(std::forward_like<T>(std::declval<U>()));

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
		auto Get(this auto&& self) -> decltype(auto)
		{
			return std::get<T>(self.Value);
		}

		template<size_t Index>
		auto Get(this auto&& self) -> decltype(auto)
		{
			return std::get<Index>(self.Value);
		}
	};

	//
	// A type sequence of compile-time types.
	template<typename...TTypes>
	struct TypeSequence
	{
		using TupleType = std::tuple<TTypes...>;
		using VariantType = std::variant<const TTypes*...>;

		constexpr void Find(this auto&& self, auto&& filterFn, auto&&...invocable)
		{
			static_assert((std::invocable<decltype(filterFn), TTypes> or ...), "The filter function must be invocable with at least one type in the sequence.");
			static_assert(sizeof...(invocable) > 0, "You must pass at least one invocable.");
			[] <typename...TArgs>(std::tuple<TArgs...>&tuple, auto&& filterFn, auto&&...invocable)
			{
				// Returns false if the filter function cannot be invoked on the 
				// tuple element type, or the overload does not accept the tuple
				// element type. This allows client code to specify filter and
				// and invocable functions with only what they care about.
				auto overload = Overload{ std::forward<decltype(invocable)>(invocable)... };
				(... or [&filterFn, &overload, &tuple]<typename TArg = TArgs>
				{
					if constexpr (not std::invocable<decltype(filterFn), TArg>)
						return false;
					else if constexpr (not std::invocable<decltype(overload), decltype(std::get<TArg>(tuple))>)
						return false;
					else return std::invoke(filterFn, std::get<TArg>(tuple))
						? (std::invoke(overload, std::get<TArg>(tuple)), true)
						: false;
				}());
			}(self.AllTypes, std::forward<decltype(filterFn)>(filterFn), std::forward<decltype(invocable)>(invocable)...);
		}

		constexpr void RunAll(this auto&& self, auto&&...invocable)
		{
			[] <typename...TArgs>(std::tuple<TArgs...> &tuple, auto&& overload)
			{
				([&overload, &tuple]<typename T = TArgs>
				{
					//if constexpr (requires { std::invoke(overload, std::get<T>(tuple)); })
					if constexpr (std::invocable<decltype(overload), T&>)
						std::invoke(overload, std::get<T>(tuple));
				}(), ...);
			}(self.AllTypes, Overload{ std::forward<decltype(invocable)>(invocable)... });
		}

		constexpr void RunOne(this auto&& self, auto&& invocable)
		{
			[] <typename...TArgs>(std::tuple<TArgs...>&tuple, auto&& invocable)
			{
				([&invocable, &tuple]<typename T = TArgs>
				{
					//if constexpr (requires { std::invoke(invocable, std::get<T>(tuple)); })
					if constexpr (std::invocable<decltype(invocable), T&>)
						return (std::invoke(invocable, std::get<T>(tuple)), true);
					else
						return false;
				}() or ...);
			}(self.AllTypes, invocable);
		}

		template<typename T>
		constexpr auto GetByType(this auto&& self) noexcept -> decltype(auto)
		{
			return std::get<T>(self.AllTypes);
		}

		template<unsigned Index>
		constexpr auto GetByIndex(this auto&& self) noexcept -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(std::get<Index>(self.AllTypes));
		}

		TupleType AllTypes;
	};
}
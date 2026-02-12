//
// Created by Andrey Solovyev on 29/01/2023.
//

#pragma once

#include <type_traits>
#ifdef __cpp_concepts
#include <concepts>
#endif

namespace culib::requirements {


#ifndef __cpp_concepts
    namespace details {
	template<typename Input, typename Result, typename = void>
	struct MaybeConveribleOrConstructibleFromTo : std::false_type { };

	template<typename Input, typename Result>
	struct MaybeConveribleOrConstructibleFromTo<
			Input, Result,
			std::void_t<
					std::enable_if_t<
							std::disjunction_v<
									std::is_convertible<Input, Result>,
									std::is_constructible<Result, Input>
							>
					>
			>
	> : std::true_type {};
  }//!namespace
  template <typename Input, typename Result>
  inline constexpr bool is_converible_or_constructible_v {
	  details::MaybeConveribleOrConstructibleFromTo<Input, Result>::value };

  template <typename Input, typename Result>
  using ConveribleOrConstructibleFromTo = std::enable_if_t<is_converible_or_constructible_v<Input, Result>, bool>;

#else

    template <typename Input, typename Result>
    concept ConveribleOrConstructibleFromTo = std::convertible_to<Input, Result> || std::constructible_from<Result, Input>;

    template <typename Input, typename Result>
    constexpr inline bool is_converible_or_constructible_v { ConveribleOrConstructibleFromTo<Input, Result> };

    //todo requires C++17
    template<typename Type, typename... MaybeType>
    concept AllTheSame = requires {sizeof...(MaybeType) > 0u;} && (std::same_as<Type, MaybeType> && ...);

#endif
}//!namespace

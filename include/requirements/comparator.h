//
// Created by Andrey Solovyev on 16/02/2023.
//

#pragma once

#include <type_traits>
#include <functional>
#ifdef __cpp_concepts
#include <concepts>
#endif

namespace culib::requirements {
  /**
 * @brief
 * check that type is a comparator for another type
 * */

#ifndef __cpp_concepts

  namespace details {

	template<typename Value, typename Type, typename = void>
	struct MaybeComparator : std::false_type { };
	template<typename Value, typename Type>
	struct MaybeComparator<Value, Type,
						   std::void_t<
								   std::enable_if_t<
								   std::is_invocable_r_v<bool, MaybeComparator, std::add_const_t<std::decay_t<Type>>, std::add_const_t<std::decay_t<Type>> >>
						   >
	> : std::true_type {};
  }//!namespace

  template <typename Value, typename Type>
  inline constexpr bool is_comparator_v { details::MaybeComparator<Value, Type>::value };

  template <typename Value, typename Type>
  using IsComparator = std::enable_if_t<is_comparator_v<Value, Type>, bool>;
#else

	template<typename Type, typename MaybeComparator>
	concept IsComparator = requires() {
		requires std::is_invocable_r_v<bool, MaybeComparator, std::add_const_t<std::decay_t<Type>>, std::add_const_t<std::decay_t<Type>> >;
	};


	template <typename Type, typename MaybeComparator>
	concept NotComparator = requires() {
		requires !IsComparator<Type, MaybeComparator>;
	};

	template <typename Type, typename MaybeComparator>
	static inline constexpr bool is_comparator_v { IsComparator<Type, MaybeComparator> ? true : false };

#endif


}//!namespace

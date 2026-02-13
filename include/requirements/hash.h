//
// Created by Andrey Solovyev on 16/02/2023.
//

#pragma once

#include <cstddef>
#include <type_traits>
#include <functional>
#ifdef __cpp_concepts
#include <concepts>
#endif

namespace culib::requirements {
/**
 * @brief
 * check that type is a hash for another type
 * */
#ifndef __cpp_concepts
	namespace details {

		template<typename HashableType, typename MaybeHash, typename HashResult, typename = void>
		struct MaybeUserDefinedHash : std::false_type { };
		template<typename HashableType, typename MaybeHash, typename HashResult>
		struct MaybeUserDefinedHash<HashableType, MaybeHash, HashResult,
				std::void_t<
						std::enable_if_t<std::negation_v<std::is_same<HashResult, bool>>>,
						std::enable_if_t<std::is_invocable_r_v<HashResult, MaybeHash, std::add_const_t<std::decay_t<HashableType>>>>,
						std::enable_if_t<std::is_copy_constructible_v<MaybeHash>>,
						std::enable_if_t<std::is_move_constructible_v<MaybeHash>>
				>
		> : std::true_type {};

		template<typename HashableType, typename MaybeHash, typename = void>
		struct MaybeBuiltInHash : std::false_type { };

		template<typename HashableType, typename MaybeHash>
		struct MaybeBuiltInHash<HashableType, MaybeHash,
				std::void_t<std::enable_if_t<std::is_same_v<typename std::hash<HashableType>, MaybeHash>>>
		> : std::true_type {};

	}//!namespace

	template <typename HashableType, typename MaybeHash, typename HashResult = std::size_t>
	inline constexpr bool is_hash_v {
			details::MaybeUserDefinedHash<HashableType, MaybeHash, HashResult>::value ||
			details::MaybeBuiltInHash<HashableType, MaybeHash>::value };

	template <typename HashableType, typename MaybeHash, typename HashResult = std::size_t>
	using IsHash = std::enable_if_t<is_hash_v<HashableType, MaybeHash, HashResult>, bool>;

#else
	template <typename HashableType, typename MaybeHash, typename HashResult = std::size_t>
	concept IsHash = (
			                 std::negation_v<std::is_same<HashResult, bool>> &&
			                 std::is_invocable_r_v<HashResult, MaybeHash, std::add_const_t<std::decay_t<HashableType>>> &&
			                 std::is_copy_constructible_v<MaybeHash> &&
			                 std::is_move_constructible_v<MaybeHash>
	                 ) ||
	                 std::is_same_v<typename std::hash<HashableType>, MaybeHash>;

	template <typename Key, typename Type, typename HashResult = std::size_t>
	inline constexpr bool is_hash_v { IsHash<Key, Type, HashResult> ? true : false };

#endif

}//!namespace


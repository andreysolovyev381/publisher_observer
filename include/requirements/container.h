//
// Created by Andrey Solovyev on 16/02/2023.
//

#pragma once

#include <memory_resource>
#include <type_traits>
#ifdef __cpp_concepts
#include <concepts>
#endif

namespace culib::requirements {

#ifndef __cpp_concepts

	//todo this section requires an update to keep up with C++20 concepts - see below

	namespace details {

	template<typename Container, typename = void>
	struct MaybeContainer : std::false_type { };

	template<typename Container>
	struct MaybeContainer<Container,
						  std::void_t<
								  decltype(std::declval<Container>().begin()),
								  decltype(std::declval<Container>().end()),
								  decltype(std::declval<Container>().size()),
								  decltype(std::declval<Container>().empty()),
								  typename Container::value_type
						  >
	> : std::true_type {};
  }//!namespace

  template<typename Container>
  inline constexpr bool is_container_v { details::MaybeContainer<Container>::value };

  template<typename Container>
  using IsContainer = std::enable_if_t<is_container_v<Container>, bool>;

  template <typename... Args>
  constexpr bool areAllContainers_v () {
	  bool result {true};
	  return ((result = result && is_container_v<Args>),...);
  }

  template<typename... MaybeContainer>
  using AreAllContainers = std::enable_if_t<areAllContainers_v<MaybeContainer...>(), bool>;

  template<typename... MaybeContainer>
  using NotAreAllContainers = std::enable_if_t<!areAllContainers_v<MaybeContainer...>(), bool>;

  template<typename Element>
  using IsDefaultConstructible = std::enable_if_t<std::is_default_constructible_v<Element>, bool>;


	namespace details {

		template <typename Container, typename = void>
		struct MaybePmrConstructible : std::false_type { };

		template<typename Container>
		struct MaybePmrConstructible<Container,
				std::void_t<
						std::is_constructible<Container, std::pmr::memory_resource*>,
						typename Container::allocator_type,
						decltype(std::declval<Container>().get_allocator())
				>
		> : std::true_type {};

	}//!namespace

	template<typename Container>
	inline constexpr bool is_pmr_constructible_v { is_container_v<Container> && details::MaybePmrConstructible<Container>::value };

	template<typename Container>
	using IsPmrConstructible = std::enable_if_t<is_pmr_constructible_v<Container>, bool>;


#else

	template <typename C>
	concept HasValueType = requires () {
		typename std::remove_cvref_t<C>::value_type;
	};
	template <typename C> static constexpr inline bool has_value_type_v{HasValueType<C>};

	template <typename C> 
	concept HasMappedType = requires () { 
		typename std::remove_cvref_t<C>::mapped_type; 
	};
	template <typename C> static constexpr inline bool has_mapped_type_v{HasMappedType<C>};

	template <typename C> 
	concept HasKeyType = requires () { 
		typename std::remove_cvref_t<C>::key_type; 
	};
	template <typename C> static constexpr inline bool has_key_type_v{HasKeyType<C>};

	template <typename C>
	concept HasIterator = requires () {
		typename std::remove_cvref_t<C>::iterator;
	};
	template <typename C> static constexpr inline bool has_iterator_v{HasIterator<C>};

	template <typename C>
	concept HasConstIterator = requires () {
		typename std::remove_cvref_t<C>::const_iterator;
	};
	template <typename C> static constexpr inline bool has_const_iterator_v{HasConstIterator<C>};

	template <typename C>
	concept HasDataMethod = requires (std::remove_cvref_t<C> c) {
		c.data();
		requires std::is_pointer_v <decltype(c.data())>;
	};
	template <typename C> static constexpr inline bool has_data_method_v{HasDataMethod<C>};

	template <typename C>
	concept HasSizeMethod = requires (std::remove_cvref_t<C> c) {
		{ c.size() } -> std::same_as<std::size_t>;
	};
	template <typename C> static constexpr inline bool has_size_method_v{HasSizeMethod<C>};

	//std::map and other node based containers don't have ::data() method
	template<typename C>
    concept IsContainer = 
		HasValueType<C> &&
		HasIterator<C> &&
		HasConstIterator<C> &&
		HasSizeMethod<C> &&
		requires (std::remove_cvref_t<C> c) {
        	{ c.begin() } -> std::convertible_to<typename std::remove_cvref_t<C>::iterator>;
        	{ c.end() } -> std::convertible_to<typename std::remove_cvref_t<C>::iterator>;
        	{ c.empty() } -> std::same_as<bool>;
		};

	template<typename MaybeContainer>
	inline constexpr bool is_container_v { IsContainer<MaybeContainer> };



	template<typename... MaybeContainer>
	concept AreAllContainers = (IsContainer<MaybeContainer> && ...);

	template<typename... MaybeContainer>
	inline constexpr bool are_containers_v { (IsContainer<MaybeContainer> && ...) };


	template<typename C>
	concept IsDict = IsContainer<C> && HasKeyType<C> && HasMappedType<C>;
	template<typename C>
	static constexpr inline bool is_dict_v {IsDict<C>};


	template<typename C>
	concept IsArray = IsContainer<C> &&
	        std::same_as<C, std::array<typename std::decay_t<C>::value_type, std::extent_v<C>>>;
	template<typename C>
	static constexpr inline bool is_array_v {IsArray<C>};


	template<typename C>
	concept ContainerHasInsert = IsContainer<C> &&
	                  requires (C c, typename std::decay_t<C>::const_iterator it, typename std::decay_t<C>::value_type value) {
		{ c.insert(it, std::move(value)) } -> std::same_as<typename std::decay_t<C>::iterator>;
	};
	template<typename C>
	static constexpr inline bool has_insert_v {ContainerHasInsert<C>};

	template<template <typename...> typename C, typename... Args>
	concept IsInstantiable = requires { typename C<Args...>; };

	template<template <typename...> typename C, typename... Args>
	inline constexpr bool is_instantiable_v { IsInstantiable<C, Args...> };

	template<template <typename...> typename C, typename... Args>
	concept IsContainerTemplate = IsInstantiable<C, Args...> && IsContainer<C<Args...>>;

	template<template <typename...> typename C, typename... Args>
	inline constexpr bool is_container_template_v { IsContainerTemplate<C, Args...> };



	template <typename T>
	concept is_pmr_constructible =
	std::constructible_from<T, std::pmr::memory_resource*> &&
	requires (T c)
	{
		typename T::allocator_type;
		{ c.get_allocator() } noexcept;
	};

#endif

}//!namespace

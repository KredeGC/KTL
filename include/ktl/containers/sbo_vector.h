#pragma once

#include "../utility/assert.h"
#include "../utility/unreachable.h"
#include "sbo_vector_fwd.h"

#include "inline_vector.h"

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>
#include <variant>
#include <type_traits>

namespace ktl
{
	template<typename Vec>
	class sbo_vector
	{
	public:
		using value_type = typename Vec::value_type;

	private:
		static_assert(std::is_default_constructible<value_type>::value, "Template class needs to be default constructible");

		static constexpr size_t InlineCapacityInBytes = sizeof(Vec) - sizeof(size_t);
		static constexpr size_t InlineCapacity = (InlineCapacityInBytes + sizeof(value_type) - 1) / sizeof(value_type);

		using inline_vec = inline_vector<value_type, InlineCapacity>;
		using variant_type = std::variant<inline_vec, Vec>;

	public:
		using size_type = typename Vec::size_type;
		using difference_type = std::ptrdiff_t;
		using allocator_type = typename Vec::allocator_type;

		using reference = value_type&;
		using const_reference = const value_type&;

		using iterator = value_type*;
		using const_iterator = const value_type*;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static_assert(std::is_same_v<reference, typename Vec::reference>, "Vector class must use T& as reference type");

	public:
		/**
		 * @brief Construct the vector with a default constructed allocator
		*/
		sbo_vector() noexcept :
			m_Data(std::in_place_type<inline_vec>, 0) {}

		/**
		 * @brief Construct the vector with the given allocator
		 * @param allocator The allocator to use
		*/
		explicit sbo_vector(const allocator_type& alloc) noexcept :
			m_Data(std::in_place_type<Vec>, alloc) {}

		/**
		 * @brief Construct the vector with the vector
		 * @param vec The vector to use
		*/
		explicit sbo_vector(const Vec& vec) noexcept :
			m_Data(std::in_place_type<Vec>, vec) {}

		/**
		 * @brief Construct the vector with the given vector
		 * @param vec The vector to use
		*/
		explicit sbo_vector(Vec&& vec) noexcept :
			m_Data(std::in_place_type<Vec>, std::move(vec)) {}

		/**
		 * @brief Construct the vector with the given allocator and initial size
		 * @param n The initial size of the vector
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit sbo_vector(size_t n, const allocator_type& allocator = allocator_type()) :
			m_Data(n > InlineCapacity
				? variant_type{ std::in_place_type<Vec>, n, allocator }
				: variant_type{ std::in_place_type<inline_vec>, n })
		{}

		/**
		 * @brief Construct the vector with the given allocator, initial size and default value
		 * @param n The initial size of the vector
		 * @param value The value to initialize every element as
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit sbo_vector(size_t n, const value_type& value, const allocator_type& allocator = allocator_type()) :
			m_Data(n > InlineCapacity
				? variant_type{ std::in_place_type<Vec>, n, value, allocator }
				: variant_type{ std::in_place_type<inline_vec>, n, value })
		{}

		/**
		 * @brief Construct the vector with the allocator and range of values
		 * @param initializer The initial set of values
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		sbo_vector(std::initializer_list<value_type> initializer, const allocator_type& allocator = allocator_type()) :
			m_Data(initializer.size() > InlineCapacity
				? variant_type{ std::in_place_type<Vec>, initializer, allocator }
				: variant_type{ std::in_place_type<inline_vec>, initializer })
		{}

		/**
		 * @brief Construct the vector with the allocator and range of values
		 * @param first A pointer to the first element
		 * @param last A pointer past the last element
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit sbo_vector(const_iterator first, const_iterator last, const allocator_type& allocator = allocator_type()) :
			m_Data((last - first) > InlineCapacity
				? variant_type{ std::in_place_type<Vec>, first, last, allocator }
				: variant_type{ std::in_place_type<inline_vec>, first, last })
		{}

		friend bool operator==(const sbo_vector& lhs, const sbo_vector& rhs) noexcept
		{
			if (lhs.m_Data.index() != rhs.m_Data.index())
			{
				if (lhs.size() != rhs.size())
					return false;

				if constexpr (std::is_trivial_v<value_type>)
				{
					return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
				}
				else
				{
					for (size_t i = 0; i < lhs.size(); ++i)
					{
						if (lhs.at(i) != rhs.at(i))
							return false;
					}

					return true;
				}
			}

			switch (lhs.m_Data.index())
			{
			case 0:
				return std::get<inline_vec>(lhs.m_Data) == std::get<inline_vec>(rhs.m_Data);
			case 1:
				return std::get<Vec>(lhs.m_Data) == std::get<Vec>(rhs.m_Data);
			default:
				KTL_UNREACHABLE();
			}
		}

		friend bool operator!=(const sbo_vector& lhs, const sbo_vector& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		reference operator[](size_t index) noexcept
		{
			return std::visit([&index](auto& v) -> reference { return v[index]; }, m_Data);
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		const_reference operator[](size_t index) const noexcept
		{
			return std::visit([&index](auto& v) -> const_reference { return v[index]; }, m_Data);
		}


		iterator begin() noexcept
		{
			return std::visit([this](auto& v) { return to_inline_iterator(v.begin()); }, m_Data);
		}

		const_iterator begin() const noexcept
		{
			return std::visit([this](auto& v) { return to_inline_iterator(v.begin()); }, m_Data);
		}

		iterator end() noexcept
		{
			return std::visit([this](auto& v) { return to_inline_iterator(v.end()); }, m_Data);
		}

		const_iterator end() const noexcept
		{
			return std::visit([this](auto& v) { return to_inline_iterator(v.end()); }, m_Data);
		}

		reverse_iterator rbegin() noexcept
		{
			return std::visit([this](auto& v) { return to_reverse_iterator(v.rbegin()); }, m_Data);
		}

		const_reverse_iterator rbegin() const noexcept
		{
			return std::visit([this](auto& v) { return to_reverse_iterator(v.rbegin()); }, m_Data);
		}

		reverse_iterator rend() noexcept
		{
			return std::visit([this](auto& v) { return to_reverse_iterator(v.rend()); }, m_Data);
		}

		const_reverse_iterator rend() const noexcept
		{
			return std::visit([this](auto& v) { return to_reverse_iterator(v.rend()); }, m_Data);
		}

		reference front() noexcept
		{
			return std::visit([](auto& v) -> reference { return v.front(); }, m_Data);
		}

		const_reference front() const noexcept
		{
			return std::visit([](auto& v) -> const_reference { return v.front(); }, m_Data);
		}

		reference back() noexcept
		{
			return std::visit([](auto& v) -> reference { return v.back(); }, m_Data);
		}

		const_reference back() const noexcept
		{
			return std::visit([](auto& v) -> const_reference { return v.back(); }, m_Data);
		}


		/**
		 * @brief Returns the current size of the vector.
		 * @return The current size of the vector in number of elements.
		*/
		size_type size() const noexcept
		{
			return std::visit([](auto& v) { return v.size(); }, m_Data);
		}

		/**
		 * @brief Returns the current capacity of the vector.
		 * @return The current capacity of the vector in number of elements.
		*/
		size_type capacity() const noexcept
		{
			return std::visit([](auto& v) { return v.capacity(); }, m_Data);
		}

		/**
		 * @brief Returns true if the vector has no elements.
		 * @return Whether the vector has a size of 0.
		*/
		bool empty() const noexcept
		{
			return std::visit([](auto& v) { return v.empty(); }, m_Data);
		}


		/**
		 * @brief Returns an iterator to the start of the vector.
		 * @return An iterator to the start of the vector.
		*/
		iterator data() noexcept
		{
			return std::visit([](auto& v) { return v.data(); }, m_Data);
		}

		/**
		 * @brief Returns a const iterator to the start of the vector.
		 * @return A const iterator to the start of the vector.
		*/
		const_iterator data() const noexcept
		{
			return std::visit([](auto& v) { return v.data(); }, m_Data);
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the vector. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		reference at(size_t index) noexcept
		{
			return std::visit([&index](auto& v) -> reference { return v.at(index); }, m_Data);
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the vector. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		const_reference at(size_t index) const noexcept
		{
			return std::visit([&index](auto& v) -> const_reference { return v.at(index); }, m_Data);
		}


		/**
		 * @brief Resizes the vector to the given size.
		 * @param n The size to resize to.
		*/
		void resize(size_t n) noexcept
		{
			move_if_growing(n);

			std::visit([&n](auto& v) { v.resize(n); }, m_Data);
		}

		/**
		 * @brief Reserves the capacity of the vector to @p n, without initializing any elements.
		 * @param n The minimum capacity of the vector.
		*/
		void reserve(size_t n) noexcept
		{
			move_if_growing(n);

			if (m_Data.index() == 1)
			{
				std::get<Vec>(m_Data).reserve(n);
			}
		}

		/**
		 * @brief Pushes a new element into the vector by copying it.
		 * @param value The element to copy into the vector.
		*/
		void push_back(const value_type& element) noexcept
		{
			move_if_growing(size() + 1);

			std::visit([&element](auto& v) { v.push_back(element); }, m_Data);
		}

		/**
		 * @brief Pushes a new element into the vector by moving it.
		 * @param value The element to move into the vector.
		*/
		void push_back(value_type&& element) noexcept
		{
			move_if_growing(size() + 1);

			std::visit([&element](auto& v) { v.push_back(std::move(element)); }, m_Data);
		}

		/**
		 * @brief Pushes a range of values into the vector.
		 * @param first A pointer to the first element.
		 * @param last A pointer one element past the last element.
		*/
		void assign(const_iterator first, const_iterator last) noexcept
		{
			move_if_growing(last - first);

			std::visit([&first, &last](auto& v) { v.assign(first, last); }, m_Data);
		}

		/**
		 * @brief Pushes a new element into the vector by constructing it.
		 * @tparam ...Args Variadic template arguments.
		 * @param ...args Any arguments to use in the construction of the element.
		 * @return A reference to the element that was added.
		*/
		template<typename... Args>
		reference emplace_back(Args&&... args) noexcept
		{
			move_if_growing(size() + 1);

			return std::visit([&](auto& v) { return v.emplace_back(std::forward<Args>(args) ...); }, m_Data);
		}

		/**
		 * @brief Inserts a new element into the vector by constructing it.
		 * @tparam ...Args Variadic template arguments.
		 * @param iter An iterator pointing to the location where the new element should be emplaced.
		 * @param ...args Any arguments to use in the construction of the element.
		 * @return An iterator to the element that was added.
		*/
		template<typename... Args>
		iterator emplace(const_iterator iter, Args&&... args) noexcept
		{
			move_if_growing(size() + 1);

			switch (m_Data.index())
			{
			case 0:
				return std::get<inline_vec>(m_Data).emplace(iter, std::forward<Args>(args) ...);
			case 1:
				return to_inline_iterator(std::get<Vec>(m_Data).emplace(to_vector_iterator(iter), std::forward<Args>(args) ...));
			default:
				KTL_UNREACHABLE();
			}
		}

		/**
		 * @brief Erases the element pointed to by the iterator.
		 * @param iter An iterator pointing to the element.
		 * @return An iterator pointing to the element immidiately after the erased one.
		*/
		iterator erase(const_iterator iter) noexcept
		{
			switch (m_Data.index())
			{
			case 0:
				return std::get<inline_vec>(m_Data).erase(iter);
			case 1:
				return to_inline_iterator(std::get<Vec>(m_Data).erase(to_vector_iterator(iter)));
			default:
				KTL_UNREACHABLE();
			}
		}

		/**
		 * @brief Erases all elements in a range.
		 * @param first An iterator pointing to the first element.
		 * @param last An iterator pointing to the location after the last element.
		 * @return An iterator pointing to the element immidiately after the erased ones.
		*/
		iterator erase(const_iterator first, const_iterator last) noexcept
		{
			switch (m_Data.index())
			{
			case 0:
				return std::get<inline_vec>(m_Data).erase(first, last);
			case 1:
				return to_inline_iterator(std::get<Vec>(m_Data).erase(to_vector_iterator(first), to_vector_iterator(last)));
			default:
				KTL_UNREACHABLE();
			}
		}

		/**
		 * @brief Removes the last element from the vector and returns it.
		*/
		void pop_back() noexcept
		{
			switch (m_Data.index())
			{
			case 0:
				std::get<0>(m_Data).pop_back();
				break;
			case 1:
				std::get<1>(m_Data).pop_back();
				break;
			default:
				KTL_UNREACHABLE();
			}
		}

		/**
		 * @brief Clears all elements in the vector.
		*/
		void clear() noexcept
		{
			return std::visit([](auto& v) { return v.clear(); }, m_Data);
		}

	private:
		void move_if_growing(size_t n) noexcept
		{
			if (m_Data.index() == 0 && n > InlineCapacity)
			{
				size_type old_size = size();

				Vec tmp{};
				tmp.reserve(n);
				for (size_t i = 0; i < old_size; ++i)
				{
					tmp.push_back(std::move(at(i)));
				}

				m_Data = std::move(tmp);
			}
		}

#pragma region Inline iterators
		iterator to_inline_iterator(iterator iter) noexcept
		{
			return iter;
		}

		const_iterator to_inline_iterator(const_iterator iter) const noexcept
		{
			return iter;
		}

		template<typename V = Vec, typename = std::enable_if_t<!std::is_same_v<iterator, typename V::iterator>>>
		iterator to_inline_iterator(typename V::iterator iter) noexcept
		{
			size_type diff = std::distance(std::get<Vec>(m_Data).begin(), iter);

			return std::get<Vec>(m_Data).data() + diff;
		}

		template<typename V = Vec, typename = std::enable_if_t<!std::is_same_v<iterator, typename V::iterator>>>
		const_iterator to_inline_iterator(typename V::const_iterator iter) const noexcept
		{
			size_type diff = std::distance(std::get<Vec>(m_Data).begin(), iter);

			return std::get<Vec>(m_Data).data() + diff;
		}
#pragma endregion

#pragma region Vector iterators
		typename Vec::const_iterator to_vector_iterator(const_iterator iter) const noexcept
		{
			size_type diff = iter - std::get<Vec>(m_Data).data();

			auto vec_iter = std::get<Vec>(m_Data).begin();
			std::advance(vec_iter, diff);

			return vec_iter;
		}
#pragma endregion

#pragma region Inline reverse iterators
		reverse_iterator to_reverse_iterator(reverse_iterator iter) noexcept
		{
			return iter;
		}

		const_reverse_iterator to_reverse_iterator(const_reverse_iterator iter) const noexcept
		{
			return iter;
		}

		template<typename V = Vec, typename = std::enable_if_t<!std::is_same_v<reverse_iterator, typename V::reverse_iterator>>>
		reverse_iterator to_reverse_iterator(typename V::reverse_iterator iter) noexcept
		{
			difference_type diff = std::distance(std::get<Vec>(m_Data).rbegin(), iter);

			auto inline_iter = std::reverse_iterator(to_inline_iterator(std::get<Vec>(m_Data).end()));
			std::advance(inline_iter, diff);

			return inline_iter;
		}

		template<typename V = Vec, typename = std::enable_if_t<!std::is_same_v<reverse_iterator, typename V::reverse_iterator>>>
		const_reverse_iterator to_reverse_iterator(typename V::const_reverse_iterator iter) const noexcept
		{
			difference_type diff = std::distance(std::get<Vec>(m_Data).rbegin(), iter);

			auto inline_iter = std::reverse_iterator(to_inline_iterator(std::get<Vec>(m_Data).end()));
			std::advance(inline_iter, diff);

			return inline_iter;
		}
#pragma endregion

	private:
		variant_type m_Data;
	};
}
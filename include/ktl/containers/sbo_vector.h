#pragma once

#include "../utility/assert.h"
#include "sbo_vector_fwd.h"

#include "inline_vector.h"

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>
#include <variant>

namespace ktl
{
	template<typename T, typename Vec>
	class sbo_vector
	{
	private:
		static_assert(std::is_default_constructible<T>::value, "Template class needs to be default constructible");

		static constexpr size_t InlineCapacityInBytes = sizeof(Vec) - sizeof(size_t);
		static constexpr size_t InlineCapacity = (InlineCapacityInBytes + sizeof(T) - 1) / sizeof(T);

		using inline_vec = inline_vector<T, InlineCapacity>;
		using variant_type = std::variant<inline_vec, Vec>;

	public:
		using value_type = typename Vec::value_type;
		using size_type = typename Vec::size_type;
		using difference_type = typename Vec::difference_type;
		using allocator_type = typename Vec::allocator_type;

		using reference = typename Vec::reference;
		using const_reference = typename Vec::const_reference;

		using iterator = typename Vec::iterator;
		using const_iterator = typename Vec::const_iterator;

		using reverse_iterator = typename Vec::reverse_iterator;
		using const_reverse_iterator = typename Vec::const_reverse_iterator;

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
		explicit sbo_vector(const Vec& vec) noexcept :
			m_Data(std::in_place_type<Vec>, vec) {}

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
		explicit sbo_vector(size_t n, const T& value, const allocator_type& allocator = allocator_type()) :
			m_Data(n > InlineCapacity
				? variant_type{ std::in_place_type<Vec>, n, value, allocator }
				: variant_type{ std::in_place_type<inline_vec>, n, value })
		{}

		/**
		 * @brief Construct the vector with the allocator and range of values
		 * @param initializer The initial set of values
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		sbo_vector(std::initializer_list<T> initializer, const allocator_type& allocator = allocator_type()) :
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
		explicit sbo_vector(const T* first, const T* last, const allocator_type& allocator = allocator_type()) :
			m_Data((last - first) > InlineCapacity
				? variant_type{ std::in_place_type<Vec>, first, last, allocator }
				: variant_type{ std::in_place_type<inline_vec>, first, last })
		{}

		sbo_vector(const sbo_vector& other) noexcept :
			m_Data(other.m_Data) {}

		sbo_vector(sbo_vector&& other) noexcept :
			m_Data(std::move(other.m_Data)) {}

		sbo_vector(const sbo_vector& other, const allocator_type& allocator) noexcept :
			m_Data(other.m_Data)
		{
			
		}

		sbo_vector(sbo_vector&& other, const allocator_type& allocator) noexcept :
			m_Data(std::move(other.m_Data))
		{
			
		}

		~sbo_vector() noexcept
		{
			
		}

		sbo_vector& operator=(const sbo_vector& other) noexcept
		{
			m_Data = other.m_Data;

			return *this;
		}

		sbo_vector& operator=(sbo_vector&& other) noexcept
		{
			m_Data = std::move(other.m_Data);

			return *this;
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		T& operator[](size_t index) noexcept
		{
			return std::visit([&index](auto& v) -> T& { return v[index]; }, m_Data);
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		const T& operator[](size_t index) const noexcept
		{
			return std::visit([&index](auto& v) { return v[index]; }, m_Data);
		}


		iterator begin() noexcept
		{
			return std::visit([](auto& v) { return v.begin(); }, m_Data);
		}

		const_iterator begin() const noexcept
		{
			return std::visit([](auto& v) { return v.begin(); }, m_Data);
		}

		iterator end() noexcept
		{
			return std::visit([](auto& v) { return v.end(); }, m_Data);
		}

		const_iterator end() const noexcept
		{
			return std::visit([](auto& v) { return v.end(); }, m_Data);
		}

		reverse_iterator rbegin() noexcept
		{
			return std::visit([](auto& v) { return v.rbegin(); }, m_Data);
		}

		const_reverse_iterator rbegin() const noexcept
		{
			return std::visit([](auto& v) { return v.rbegin(); }, m_Data);
		}

		reverse_iterator rend() noexcept
		{
			return std::visit([](auto& v) { return v.rend(); }, m_Data);
		}

		const_reverse_iterator rend() const noexcept
		{
			return std::visit([](auto& v) { return v.rend(); }, m_Data);
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
			return std::visit([&index](auto& v) -> const const_reference { return v.at(index); }, m_Data);
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
			//std::visit([&n](auto& v) { v.reserve(n); }, m_Data);
		}

		/**
		 * @brief Pushes a new element into the vector by copying it.
		 * @param value The element to copy into the vector.
		*/
		void push_back(const T& element) noexcept
		{
			move_if_growing(size() + 1);

			std::visit([&element](auto& v) { v.push_back(element); }, m_Data);
		}

		/**
		 * @brief Pushes a new element into the vector by moving it.
		 * @param value The element to move into the vector.
		*/
		void push_back(T&& element) noexcept
		{
			move_if_growing(size() + 1);

			std::visit([&element](auto& v) { v.push_back(std::move(element)); }, m_Data);
		}

		/**
		 * @brief Pushes a range of values into the vector.
		 * @param first A pointer to the first element.
		 * @param last A pointer one element past the last element.
		*/
		void assign(const T* first, const T* last) noexcept
		{
			move_if_growing(last - first);

			std::visit([&first, &last](auto& v) { v.assign(first, last); }, m_Data);
		}

		/**
		 * @brief Pushes a new element into the vector by constructing it.
		 * @tparam ...Args Variadic template arguments.
		 * @param ...args Any arguments to use in the construction of the element.
		 * @return An iterator to the element that was added.
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

			// TODO: Reverse this. I don't like using native std::vector<T>::iterator
			if (m_Data.index() == 0)
			{
				size_type diff = iter - begin();

				T* p = std::get<inline_vec>(m_Data).emplace(&(*iter), std::forward<Args>(args) ...);

				iterator i = begin();
				std::advance(i, diff);

				return i;
			}
			else
			{
				return std::get<Vec>(m_Data).emplace(iter, std::forward<Args>(args) ...);
			}
		}

		/**
		 * @brief Erases the element pointed to by the iterator.
		 * @param iter An iterator pointing to the element.
		 * @return An iterator pointing to the element immidiately after the erased one.
		*/
		iterator erase(const_iterator iter) noexcept
		{
			return std::visit([&iter](auto& v) { return v.erase(iter); }, m_Data);
		}

		/**
		 * @brief Erases all elements in a range.
		 * @param first An iterator pointing to the first element.
		 * @param last An iterator pointing to the location after the last element.
		 * @return An iterator pointing to the element immidiately after the erased ones.
		*/
		iterator erase(const_iterator first, const_iterator last) noexcept
		{
			return std::visit([&first, &last](auto& v) { return v.erase(first, last); }, m_Data);
		}

		/**
		 * @brief Removes the last element from the vector and returns it.
		 * @return The last element in the vector.
		*/
		T pop_back() noexcept
		{
			return std::visit([](auto& v) { return v.pop_back(); }, m_Data);
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

	private:
		variant_type m_Data;
	};
}
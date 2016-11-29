#pragma once

#include <atomic>
#include <memory>

#include <iostream>
template <typename T> class RingBuffer {
public:

	explicit RingBuffer(std::size_t capacity) : _buffer_capacity(capacity), _ring_buffer_array(new T[capacity]) {

	}

	~RingBuffer() {

	}

	template<typename U>
	void Push(U && val)
	{
		while (!TryPush(std::forward<U>(val)));
	}

	T Pop() {
		T val;
		while (!TryPop(&val));
		return val;
	}

private:

	//Private Struct to ensure allocated memory is freed.
	struct RingBufferFree {
		void operator()(T * ring_buffer_memory) { delete[] ring_buffer_memory; }
	};

	//Private Member Functions
	template<typename U>
	bool TryPush(U && val) {
		const std::size_t current_write = write_position.load(std::memory_order_acquire);
		const std::size_t current_read = read_position.load(std::memory_order_acquire);
		const std::size_t next_write = increment_index(current_write);

		if (next_write == current_read) { return false; }

		_ring_buffer_array[current_write] = std::move(val);
		write_position.store(next_write, std::memory_order_release);

		return true;
	}

	bool TryPop(T * val) {
		const std::size_t current_write = write_position.load(std::memory_order_acquire);
		const std::size_t current_read = read_position.load(std::memory_order_acquire);

		if (current_read == current_write) { return false; }

		*val = std::move(_ring_buffer_array[current_read]);

		const std::size_t next_read = increment_index(current_read);
		read_position.store(next_read, std::memory_order_release);

		return true;
	}

	std::size_t increment_index(std::size_t index) {
		return (index + 1) % _buffer_capacity;
	}

	//Private Member Variables
	std::atomic<std::size_t> read_position = 0;
	std::atomic<std::size_t> write_position = 0;

	std::size_t _buffer_capacity;
	std::unique_ptr<T[], RingBufferFree> _ring_buffer_array;
};
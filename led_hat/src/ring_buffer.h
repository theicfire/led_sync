#pragma once

// From
// https://raw.githubusercontent.com/embeddedartistry/embedded-resources/master/examples/cpp/circular_buffer.cpp
#include <unistd.h>

#include <cstdio>
const int MAX_SIZE = 100;

template <class T>
class RingBuffer {
 public:
  // TODO getter?
  T buf_[MAX_SIZE];

  explicit RingBuffer(size_t size) : max_size_(MAX_SIZE) {
    full_ = false;
    head_ = 0;
    tail_ = 0;
  }

  void put(T item) {
    buf_[head_] = item;

    if (full_) {
      tail_ = (tail_ + 1) % max_size_;
    }

    head_ = (head_ + 1) % max_size_;

    full_ = head_ == tail_;
  }

  T get() {
    if (empty()) {
      return T();
    }

    // Read data and advance the tail (we now have a free space)
    T val = buf_[tail_];
    full_ = false;
    tail_ = (tail_ + 1) % max_size_;

    return val;
  }

  void reset() {
    head_ = tail_;
    full_ = false;
  }

  bool empty() const {
    // if head and tail are equal, we are empty
    return (!full_ && (head_ == tail_));
  }

  bool full() const {
    // If tail is ahead the head by 1, we are full
    return full_;
  }

  size_t capacity() const { return max_size_; }

  size_t size() const {
    size_t size = max_size_;

    if (!full_) {
      if (head_ >= tail_) {
        size = head_ - tail_;
      } else {
        size = max_size_ + head_ - tail_;
      }
    }

    return size;
  }

  void copy(T* dest) {
    for (size_t i = 0; i < max_size_; i++) {
      dest[i] = buf_[i];
    }
  }

 private:
  size_t head_;
  size_t tail_;
  const size_t max_size_;
  bool full_;
};

// int main(void) {
// RingBuffer<uint32_t> circle(10);
// printf("\n === CPP Circular buffer check ===\n");
// printf("Size: %zu, Capacity: %zu\n", circle.size(), circle.capacity());

// uint32_t x = 1;
// printf("Put 1, val: %d\n", x);
// circle.put(x);

// x = circle.get();
// printf("Popped: %d\n", x);

// printf("Empty: %d\n", circle.empty());

// printf("Adding %zu values\n", circle.capacity() - 1);
// for (uint32_t i = 0; i < circle.capacity() - 1; i++) {
// circle.put(i);
//}

// circle.reset();

// printf("Full: %d\n", circle.full());

// printf("Adding %zu values\n", circle.capacity());
// for (uint32_t i = 0; i < circle.capacity(); i++) {
// circle.put(i);
//}

// printf("Full: %d\n", circle.full());

// printf("Reading back values: ");
// while (!circle.empty()) {
// printf("%u ", circle.get());
//}
// printf("\n");

// printf("Adding 15 values\n");
// for (uint32_t i = 0; i < circle.size() + 5; i++) {
// circle.put(i);
//}

// printf("Full: %d\n", circle.full());

// printf("Reading back values: ");
// while (!circle.empty()) {
// printf("%u ", circle.get());
//}
// printf("\n");

// printf("Empty: %d\n", circle.empty());
// printf("Full: %d\n", circle.full());

// return 0;
//}

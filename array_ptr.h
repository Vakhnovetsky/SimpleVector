#pragma once

#include <utility>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(std::size_t size) {
        if (size > 0) {
            raw_ptr_ = new Type[size]{};
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    ArrayPtr(const ArrayPtr&) = delete;

    ArrayPtr(const ArrayPtr&& other) {
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(const ArrayPtr&& rhs) noexcept {
        if (raw_ptr_ != rhs.raw_ptr_) {
            raw_ptr_ = std::move(rhs.raw_ptr_);
        }
        return *this;
    }

    [[nodiscard]] Type* Release() noexcept {
        auto out = raw_ptr_;
        raw_ptr_ = nullptr;
        return out;
    }

    Type& operator[](std::size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](std::size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};
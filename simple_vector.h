#pragma once

#include "array_ptr.h"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) : capacity_to_reserve_(capacity_to_reserve) {
    }
    size_t GetCapacity() {
        return capacity_to_reserve_;
    }
private:
    size_t capacity_to_reserve_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size) {
    }

    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) : items_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector(SimpleVector&& other) : items_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::move(other.begin(), other.end(), items_.Get());
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector(ReserveProxyObj obj) {
        Reserve(obj.GetCapacity());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (rhs.IsEmpty()) {
            Clear();
        }

        if (this != &rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (rhs.IsEmpty()) {
            Clear();
        }

        if (this != &rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }

        return *this;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_vector(new_capacity);
            std::move(begin(), end(), new_vector.Get());
            std::fill(&new_vector[size_], &new_vector[new_capacity], 0);
            new_vector.swap(items_);
            capacity_ = new_capacity;
        }
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(this->GetSize() > index);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(this->GetSize() > index);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= this->GetSize()) {
            using namespace std::literals;
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= this->GetSize()) {
            using namespace std::literals;
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size == size_) {
            return;
        }

        if (new_size < size_) {
            size_ = new_size;
            return;
        }

        size_t old_size = size_;
        size_ = new_size;

        if (new_size > capacity_) {
            capacity_ = new_size * 2;
        }
        ArrayPtr<Type> new_vector(capacity_);
        std::move(items_.Get(), &items_[old_size], new_vector.Get());
        for (auto it = &new_vector[old_size]; it != &new_vector[capacity_]; ++it) {
            *it = Type{};
        }
        new_vector.swap(items_);
    }


    Iterator begin() noexcept {
        return Iterator{ items_.Get() };
    }

    Iterator end() noexcept {
        return Iterator{ &items_[size_] };
    }

    ConstIterator begin() const noexcept {
        return ConstIterator{ items_.Get() };
    }

    ConstIterator end() const noexcept {
        return ConstIterator{ &items_[size_] };
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }

    void PushBack(const Type& item) {
        if (capacity_ == size_) {
            size_t new_capacity;
            if (size_ > 0) {
                new_capacity = size_ * 2;
            }
            else {
                new_capacity = 1;
            }
            ArrayPtr<Type> new_vector(new_capacity);
            std::copy(begin(), end(), new_vector.Get());
            std::fill(&new_vector[size_], &new_vector[new_capacity], 0);
            new_vector.swap(items_);
            capacity_ = new_capacity;
        }
        items_[size_] = item;
        size_++;
    }

    void PushBack(Type&& item) {
        if (capacity_ == size_) {
            size_t new_capacity;
            if (size_ > 0) {
                new_capacity = size_ * 2;
            }
            else {
                new_capacity = 1;
            }
            ArrayPtr<Type> new_vector(new_capacity);
            std::move(begin(), end(), new_vector.Get());
            std::fill(&new_vector[size_], &new_vector[new_capacity], 0);
            new_vector.swap(items_);
            capacity_ = new_capacity;
        }
        items_[size_] = std::move(item);
        size_++;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t n_pos = std::distance(cbegin(), pos);
        size_t new_capacity = capacity_;

        if (capacity_ == size_) {
            if (size_ > 0) {
                new_capacity = size_ * 2;
            }
            else {
                new_capacity = 1;
            }
        }
        ArrayPtr<Type> new_vector(new_capacity);
        std::copy(begin(), pos, new_vector.Get());
        new_vector[n_pos] = value;
        std::copy(&items_[n_pos], end(), &new_vector[n_pos + 1]);
        std::fill(&new_vector[size_ + 1], &new_vector[new_capacity], 0);

        new_vector.swap(items_);
        capacity_ = new_capacity;
        size_++;

        return &items_[n_pos];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());
        auto* p = const_cast<Iterator>(pos);
        size_t n_pos = std::distance(begin(), p);
        size_t new_capacity = capacity_;

        if (capacity_ == size_) {
            if (size_ > 0) {
                new_capacity = size_ * 2;
            }
            else {
                new_capacity = 1;
            }
        }
        ArrayPtr<Type> new_vector(new_capacity);
        std::move(begin(), p, new_vector.Get());
        new_vector[n_pos] = std::move(value);
        std::move(&items_[n_pos], end(), &new_vector[n_pos + 1]);
        std::fill(&new_vector[size_ + 1], &new_vector[new_capacity], 0);

        new_vector.swap(items_);
        capacity_ = new_capacity;
        size_++;

        return &items_[n_pos];
    }

    void PopBack() noexcept {
        assert(!this->IsEmpty());
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(!this->IsEmpty());
        assert(pos >= cbegin() && pos < cend());
        auto* p = const_cast<Iterator>(pos);
        size_t n_pos = std::distance(begin(), p);
        std::move((p + 1), end(), &items_[n_pos]);
        --size_;
        return &items_[n_pos];
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.cbegin(), rhs.cend(), lhs.cbegin(), lhs.cend());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
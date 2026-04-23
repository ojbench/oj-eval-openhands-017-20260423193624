
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <iostream>
#include <string>

#include <cstring>


namespace sjtu {

template <int N>
struct FixedString {
    char data[N];
    FixedString() { memset(data, 0, sizeof(data)); }
    FixedString(const char* s) {
        memset(data, 0, sizeof(data));
        strncpy(data, s, N - 1);
    }
    bool operator<(const FixedString& other) const {
        return strcmp(data, other.data) < 0;
    }
    bool operator==(const FixedString& other) const {
        return strcmp(data, other.data) == 0;
    }
};

template <class T>
class vector {
private:
    T* data;
    size_t _size;
    size_t _capacity;

    void reserve(size_t new_capacity) {
        if (new_capacity <= _capacity) return;
        T* new_data = (T*)operator new(sizeof(T) * new_capacity);
        for (size_t i = 0; i < _size; ++i) {
            new (new_data + i) T(data[i]);
            data[i].~T();
        }
        if (data) operator delete(data);
        data = new_data;
        _capacity = new_capacity;
    }

public:
    vector() : data(nullptr), _size(0), _capacity(0) {}
    vector(const vector& other) : data(nullptr), _size(other._size), _capacity(other._size) {
        if (_capacity > 0) {
            data = (T*)operator new(sizeof(T) * _capacity);
            for (size_t i = 0; i < _size; ++i) {
                new (data + i) T(other.data[i]);
            }
        }
    }
    ~vector() {
        for (size_t i = 0; i < _size; ++i) {
            data[i].~T();
        }
        if (data) operator delete(data);
    }
    vector& operator=(const vector& other) {
        if (this == &other) return *this;
        for (size_t i = 0; i < _size; ++i) {
            data[i].~T();
        }
        if (data) operator delete(data);
        _size = other._size;
        _capacity = other._size;
        if (_capacity > 0) {
            data = (T*)operator new(sizeof(T) * _capacity);
            for (size_t i = 0; i < _size; ++i) {
                new (data + i) T(other.data[i]);
            }
        } else {
            data = nullptr;
        }
        return *this;
    }

    void push_back(const T& value) {
        if (_size == _capacity) {
            reserve(_capacity == 0 ? 1 : _capacity * 2);
        }
        new (data + _size) T(value);
        _size++;
    }

    void pop_back() {
        if (_size > 0) {
            _size--;
            data[_size].~T();
        }
    }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    size_t size() const { return _size; }
    bool empty() const { return _size == 0; }
    void clear() {
        for (size_t i = 0; i < _size; ++i) {
            data[i].~T();
        }
        _size = 0;
    }
};

template <class T>
void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

template <class T, class Compare>
void sort(T* first, T* last, Compare comp) {
    if (first >= last) return;
    T* i = first;
    T* j = last - 1;
    T pivot = first[(last - first) / 2];
    while (i <= j) {
        while (comp(*i, pivot)) i++;
        while (comp(pivot, *j)) j--;
        if (i <= j) {
            swap(*i, *j);
            i++;
            j--;
        }
    }
    if (first < j + 1) sort(first, j + 1, comp);
    if (i < last) sort(i, last, comp);
}

template <class T>
struct Less {
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

template <class T>
void sort(T* first, T* last) {
    sort(first, last, Less<T>());
}

} // namespace sjtu

#endif

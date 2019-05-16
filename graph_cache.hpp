#ifndef __GRAPH_CACHE_HPP__
#define __GRAPH_CACHE_HPP__

#include <array>
#include <iostream>
#include <mutex>
#include <vector>


template<typename E>
class VertexCache {
private:
    std::vector<E> entries_;
    bool valid_;

    mutable std::mutex mtx;

public:
    VertexCache(): valid_(false) {};

    VertexCache(bool valid): entries_(NULL), valid_(valid) {};

    inline E& operator[](size_t idx) {
        //mtx.lock();
        E& ret = entries_[idx];
        //mtx.unlock();
        return ret;
    }

    inline size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return entries_.size();
    }

    inline bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return entries_.empty();
    }

    inline bool valid() const {
        std::lock_guard<std::mutex> lock(mtx);
        return valid_;
    }

    inline void append(E& embedding) {
        std::lock_guard<std::mutex> lock(mtx);
        if(!valid_) {
            valid_ = true;
        }
        entries_.push_back(embedding);
    }

    inline void remove(size_t idx) {
        std::lock_guard<std::mutex> lock(mtx);
        entries_.erase(entries_.begin() + idx);
    }

    inline void invalidate() {
        std::lock_guard<std::mutex> lock(mtx);
        valid_ = false;
        entries_.clear();
    }

    // Move initialization
    VertexCache(VertexCache&& other) {
        std::lock_guard<std::mutex> lock(other.mtx);
        valid_ = std::move(other.valid_);
        other.valid_ = false;
        entries_ = std::move(other.entries_);
        other.entries_ = std::vector<E>();
    }

    // Copy initialization
    VertexCache(const VertexCache& other) {
        std::lock_guard<std::mutex> lock(other.mtx);
        valid_ = other.valid_;
        entries_ = other.entries_;
    }

    // Move assignment
    VertexCache& operator = (VertexCache&& other) {
        std::lock(mtx, other.mtx);
        std::lock_guard<std::mutex> self_lock(mtx, std::adopt_lock);
        std::lock_guard<std::mutex> other_lock(other.mtx, std::adopt_lock);
        valid_ = std::move(other.valid_);
        other.valid_ = false;
        entries_ = std::move(other.entries_);
        other.entries_ = std::vector<E>();
        return *this;
    }

    // Copy assignment
    VertexCache& operator = (const VertexCache& other) {
        std::lock(mtx, other.mtx);
        std::lock_guard<std::mutex> self_lock(mtx, std::adopt_lock);
        std::lock_guard<std::mutex> other_lock(other.mtx, std::adopt_lock);
        valid_ = other.valid_;
        entries_ = other.entries_;
        return *this;
    }
};

template<typename E>
class GraphCache {
protected:
    std::vector<VertexCache<E>> cache_;

public:
    GraphCache() {};

    GraphCache(size_t capacity) {
        cache_.resize(capacity);
    };

    inline void resize (size_t capacity) {
        cache_.resize(capacity);
    }

    inline VertexCache<E>& operator[](size_t idx) {
        return cache_[idx];
    }

    inline size_t size() {
        return cache_.size();
    }

    inline void append_to_vertex(size_t idx, E& embedding) {
        cache_[idx].append(embedding);
    }

    inline void remove_from_vertex(size_t vidx, size_t idx) {
        cache_[vidx].remove(idx);
    }

    inline void invalidate(size_t idx) {
        cache_[idx].invalidate();
    }
};

template<typename E>
void print_cache(GraphCache<E>& cache) {
    for(uint32_t i = 0; i < cache.size(); ++i) {
        std::cout << "line " << i << ": ";
        if(!cache[i].valid()) {
            std::cout << "invalid";
        }
        for (uint32_t j = 0; j < cache[i].size(); ++j) {
            std::cout << cache[i][j] << ' ';
        }
        std::cout << '\n';
    }
}

/*int main()
{
    // Create a vector containing integers
    GraphCache<int> cache(4);

    print_cache(cache);

    int one = 1;
    int two = 2;
    int ten = 10;

    cache.append_to_vertex(0, one);
    cache.append_to_vertex(0, two);
    cache.append_to_vertex(1, ten);

    print_cache(cache);

    cache.remove_from_vertex(0, 1);

    print_cache(cache);

    cache.invalidate(0);

    print_cache(cache);
}*/


#endif
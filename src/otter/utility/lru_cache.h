#ifndef OTTER_UTILITY_LRU_CACHE_H
#define OTTER_UTILITY_LRU_CACHE_H

#include <cassert>
#include <functional>
#include <list>
#include <memory_resource>
#include <optional>
#include <unordered_map>
#include <utility>

#include <gsl/gsl>

namespace otter {

template<typename Key,
         typename Value,
         typename Hash = std::hash<Key>,
         typename Equal = std::equal_to<Key>>
class LRUCache {
public:
    using Resource = std::pmr::memory_resource;
    using SizeType = std::size_t;
    using ValueType = std::pair<const Key, Value>;
    using Container = std::pmr::list<ValueType>;
    using Iterator = typename Container::iterator;
    using ConstIterator = typename Container::const_iterator;
    using Cache = std::pmr::unordered_map<Key, Iterator, Hash, Equal>;

private:
    SizeType capacity_;
    Container items_;
    Cache cache_;

public:
    explicit LRUCache(SizeType capacity,
                      gsl::not_null<Resource*> memory_resource = std::pmr::get_default_resource())
      : capacity_{ capacity }
      , items_{ memory_resource.get() }
      , cache_{ memory_resource.get() }
    {
        assert(capacity_ > 0 && "LRUCache capacity must be greater than 0");
    }

    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    LRUCache(LRUCache&&) = default;
    LRUCache& operator=(LRUCache&&) = default;

    ~LRUCache() = default;

    /// @brief 向缓存中插入或更新键值对。
    /// 若键已存在，更新其值并将其移到最近使用位置。
    /// 若键不存在且缓存已满，驱逐最久未使用的条目后插入新条目。
    template<typename K, typename V>
        requires std::constructible_from<Key, K> && std::constructible_from<Value, V>
    void put(K&& key, V&& value)
    {
        if (capacity_ == 0)
            return;

        auto it = cache_.find(key);
        if (it != cache_.end()) {
            it->second->second = std::forward<V>(value);
            items_.splice(items_.begin(), items_, it->second);
            return;
        }

        if (cache_.size() == capacity_) {
            cache_.erase(items_.back().first);
            items_.pop_back();
        }

        items_.emplace_front(std::forward<K>(key), std::forward<V>(value));
        cache_.emplace(items_.begin()->first, items_.begin());
    }

    /// @brief 获取键对应的值。
    /// 若键存在，将该项移到最近使用位置。
    /// 返回的引用有效期为直到下次修改缓存为止。
    template<typename K>
    auto get(const K& key) noexcept -> std::optional<std::reference_wrapper<Value>>
    {
        auto it = cache_.find(key);
        if (it == cache_.end())
            return {};

        items_.splice(items_.begin(), items_, it->second);
        return std::ref(it->second->second);
    }

    void clear() noexcept
    {
        items_.clear();
        cache_.clear();
    }

    [[nodiscard]]
    constexpr SizeType capacity() const noexcept
    {
        return capacity_;
    }

    [[nodiscard]]
    constexpr SizeType size() const noexcept
    {
        return items_.size();
    }

    Iterator begin() noexcept
    {
        return items_.begin();
    }

    Iterator end() noexcept
    {
        return items_.end();
    }

    [[nodiscard]]
    ConstIterator begin() const noexcept
    {
        return items_.begin();
    }

    [[nodiscard]]
    ConstIterator end() const noexcept
    {
        return items_.end();
    }

    ConstIterator cbegin() noexcept
    {
        return items_.cbegin();
    }

    ConstIterator cend() noexcept
    {
        return items_.cend();
    }

    [[nodiscard]]
    ConstIterator cbegin() const noexcept
    {
        return items_.cbegin();
    }

    [[nodiscard]]
    ConstIterator cend() const noexcept
    {
        return items_.cend();
    }
};

} // namespace otter

#endif // OTTER_UTILITY_LRU_CACHE_H

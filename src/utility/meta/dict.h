#ifndef OTTER_UTILITY_META_DICT_H
#define OTTER_UTILITY_META_DICT_H

#include <cstddef>
#include <string_view>

#include "string.h"

namespace otter::meta {

template<String Key, typename Value>
struct Field {
    static constexpr auto key = Key;
    Value value;

    constexpr Field() = default;

    template<typename V>
        requires std::constructible_from<Value, V&&>
    constexpr explicit Field(V&& input)
      : value{ std::forward<V>(input) }
    {}
};

template<typename... Fields>
class Dict : Fields... {
private:
    template<String Key, typename V>
    static constexpr auto get_impl(const Field<Key, V>& field) -> decltype(auto)
    {
        return field.value;
    }

    template<String Key, typename V>
    static constexpr auto has_impl(const Field<Key, V>&) -> std::true_type;

    static constexpr auto has_impl(...) -> std::false_type;

public:
    constexpr Dict(Fields... fields)
      : Fields{ std::move(fields) }...
    {}

    template<String Key>
    constexpr auto key() const
    {
        return get_impl<Key>(*this);
    }

    template<String Key>
    constexpr auto contains() const noexcept
    {
        return requires(const Dict& dict) { get_impl<Key>(dict); };
    }

    template<String Key, typename DefaultType>
    constexpr auto get_or(DefaultType&& default_value) const
    {
        if constexpr (requires(const Dict& dict) { get_impl<Key>(dict); })
            return key<Key>();
        else
            return std::forward<DefaultType>(default_value);
    }

    template<String Key, std::size_t N>
    constexpr auto get_or(const char (&default_value)[N]) const noexcept
    {
        if constexpr (requires(const Dict& dict) { get_impl<Key>(dict); })
            return key<Key>();
        else
            return std::string_view{ default_value, N - 1 };
    }

    template<String Key, std::size_t N>
    constexpr auto get_or(char (&default_value)[N]) const noexcept
    {
        if constexpr (requires(const Dict& dict) { get_impl<Key>(dict); })
            return key<Key>();
        else
            return std::string_view{ default_value, N - 1 };
    }

    constexpr auto size() const noexcept -> std::size_t
    {
        return sizeof...(Fields);
    }
};

template<typename... Fields>
Dict(Fields...) -> Dict<Fields...>;

template<String Key>
struct SymbolTag {
    template<std::size_t N>
    constexpr auto operator=(const char (&value)[N]) const noexcept
    {
        return Field<Key, std::string_view>{ std::string_view{ value, N - 1 } };
    }

    template<std::size_t N>
    constexpr auto operator=(char (&value)[N]) const noexcept
    {
        return Field<Key, std::string_view>{ std::string_view{ value, N - 1 } };
    }

    template<typename Element, std::size_t N>
        requires(!std::same_as<std::remove_cv_t<Element>, char>)
    constexpr auto operator=(const Element (&value)[N]) const
    {
        return Field<Key, std::array<std::remove_cv_t<Element>, N>>{ std::to_array(value) };
    }

    template<typename V>
        requires(!std::is_array_v<std::remove_reference_t<V>>)
    constexpr auto operator=(V&& value) const noexcept
    {
        return Field<Key, std::remove_cvref_t<V>>{ std::forward<V>(value) };
    }
};

template<String Key, typename V>
constexpr auto sym(V&& value) noexcept(noexcept(SymbolTag<Key>{} = std::forward<V>(value)))
{
    return SymbolTag<Key>{} = std::forward<V>(value);
}

inline namespace literals {

template<String Key>
constexpr auto operator""_sym()
{
    return SymbolTag<Key>{};
}

} // namespace literals

} // namespace otter::meta

#endif // OTTER_UTILITY_META_DICT_H
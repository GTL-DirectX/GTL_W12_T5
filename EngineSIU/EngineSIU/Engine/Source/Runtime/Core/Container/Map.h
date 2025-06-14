#pragma once
#include <unordered_map>

#include "ContainerAllocator.h"
#include "Pair.h"
#include "Serialization/Archive.h"


template <typename InKeyType, typename InValueType, typename Allocator = FDefaultAllocator<std::pair<const InKeyType, InValueType>>>
class TMap
{
public:
    using KeyType = InKeyType;
    using ValueType = InValueType;

    using PairType = TPair<const KeyType, InValueType>;
    using MapType = std::unordered_map<KeyType, InValueType, std::hash<KeyType>, std::equal_to<>, Allocator>;
    using SizeType = typename MapType::size_type;

private:
    MapType ContainerPrivate;

public:
    class Iterator
    {
    private:
        typename MapType::iterator InnerIt;
    public:
        Iterator(typename MapType::iterator it) : InnerIt(std::move(it)) {}
        PairType& operator*() { return reinterpret_cast<PairType&>(*InnerIt); }
        PairType* operator->() { return reinterpret_cast<PairType*>(&(*InnerIt)); }
        Iterator& operator++() { ++InnerIt; return *this; }
        bool operator!=(const Iterator& other) const { return InnerIt != other.InnerIt; }
    };

    class ConstIterator
    {
    private:
        typename MapType::const_iterator InnerIt;
    public:
        ConstIterator(typename MapType::const_iterator it) : InnerIt(it) {}
        const PairType& operator*() const { return reinterpret_cast<const PairType&>(*InnerIt); }
        const PairType* operator->() const { return reinterpret_cast<const PairType*>(&(*InnerIt)); }
        ConstIterator& operator++() { ++InnerIt; return *this; }
        bool operator!=(const ConstIterator& other) const { return InnerIt != other.InnerIt; }
    };

public:
    // TPair를 반환하는 커스텀 반복자
    Iterator begin() noexcept { return Iterator(ContainerPrivate.begin()); }
    Iterator end() noexcept { return Iterator(ContainerPrivate.end()); }
    ConstIterator begin() const noexcept { return ConstIterator(ContainerPrivate.begin()); }
    ConstIterator end() const noexcept { return ConstIterator(ContainerPrivate.end()); }

    MapType& GetContainerPrivate() { return ContainerPrivate; }
    const MapType& GetContainerPrivate() const { return ContainerPrivate; }

    // 생성자 및 소멸자
    TMap() = default;
    ~TMap() = default;

    // 복사 생성자
    TMap(const TMap& Other) : ContainerPrivate(Other.ContainerPrivate) {}

    // 이동 생성자
    TMap(TMap&& Other) noexcept : ContainerPrivate(std::move(Other.ContainerPrivate)) {}

    // 복사 할당 연산자
    TMap& operator=(const TMap& Other)
    {
        if (this != &Other)
        {
            ContainerPrivate = Other.ContainerPrivate;
        }
        return *this;
    }

    // 이동 할당 연산자
    TMap& operator=(TMap&& Other) noexcept
    {
        if (this != &Other)
        {
            ContainerPrivate = std::move(Other.ContainerPrivate);
        }
        return *this;
    }

    // 요소 접근 및 수정
    InValueType& operator[](const KeyType& Key)
    {
        return ContainerPrivate[Key];
    }

    const InValueType& operator[](const KeyType& Key) const
    {
        return ContainerPrivate.at(Key);
    }

    void Add(const KeyType& Key, const InValueType& Value)
    {
        ContainerPrivate.insert_or_assign(Key, Value);
    }

    /**
     * Map에 새로운 Key-Value를 삽입합니다.
     * @param InKey 삽입할 키
     * @param InValue 삽입할 값
     * @return InValue의 참조
     */
    template <typename InitKeyType = KeyType, typename InitValueType = InValueType>
    InValueType& Emplace(InitKeyType&& InKey, InitValueType&& InValue)
    {
        auto result = ContainerPrivate.emplace(std::forward<InitKeyType>(InKey), std::forward<InitValueType>(InValue));
        return result.first->second;
    }

    // Key만 넣고, Value는 기본값으로 삽입
    template <typename InitKeyType = KeyType>
    InValueType& Emplace(InitKeyType&& InKey)
    {
        auto result = ContainerPrivate.emplace(std::forward<InitKeyType>(InKey), InValueType{});
        return result.first->second;
    }

    void Remove(const KeyType& Key)
    {
        ContainerPrivate.erase(Key);
    }

    void Empty()
    {
        ContainerPrivate.clear();
    }

    void Empty(SizeType Number)
    {
        ContainerPrivate.clear();
        ContainerPrivate.reserve(Number);
    }

    // 검색 및 조회
    bool Contains(const KeyType& Key) const
    {
        return ContainerPrivate.contains(Key);
    }

    const InValueType* Find(const KeyType& Key) const
    {
        auto it = ContainerPrivate.find(Key);
        return it != ContainerPrivate.end() ? &(it->second) : nullptr;
    }

    InValueType* Find(const KeyType& Key)
    {
        auto it = ContainerPrivate.find(Key);
        return it != ContainerPrivate.end() ? &(it->second) : nullptr;
    }

    InValueType& FindOrAdd(const KeyType& Key)
    {
        if (InValueType* Value = Find(Key))
        {
            return *Value;
        }
        return Emplace(Key);
    }

    InValueType FindRef(const KeyType& Key) const
    {
        auto it = ContainerPrivate.find(Key);
        if (it != ContainerPrivate.end())
        {
            return it->second;
        }
        return InValueType{}; // 기본값 반환
    }

    // 크기 관련
    SizeType Num() const
    {
        return ContainerPrivate.size();
    }

    bool IsEmpty() const
    {
        return ContainerPrivate.empty();
    }

    // 용량 관련
    void Reserve(SizeType Number)
    {
        ContainerPrivate.reserve(Number);
    }
};

template <typename KeyType, typename ValueType, typename Allocator>
FArchive& operator<<(FArchive& Ar, TMap<KeyType, ValueType, Allocator>& Map)
{
    using SizeType = typename TMap<KeyType, ValueType, Allocator>::SizeType;

    // 맵 크기 직렬화
    SizeType MapSize = Map.Num();
    Ar << MapSize;

    if (Ar.IsLoading())
    {
        // 로드 시 맵 초기화
        Map.Empty(MapSize);

        for (SizeType i = 0; i < MapSize; ++i)
        {
            KeyType TempKey;
            ValueType TempValue;
            Ar << TempKey;
            Ar << TempValue;

            Map.Emplace(std::move(TempKey), std::move(TempValue));
        }
    }
    else
    {
        // 맵의 각 키-값 쌍 직렬화
        for (auto& [Key, Value] : Map)
        {
            Ar << const_cast<KeyType&>(Key);
            Ar << Value;
        }
    }

    return Ar;
}

template <typename T> constexpr bool TIsTMap_V = false;

template <typename KeyType, typename ValueType, typename Allocator> constexpr bool TIsTMap_V<               TMap<KeyType, ValueType, Allocator>> = true;
template <typename KeyType, typename ValueType, typename Allocator> constexpr bool TIsTMap_V<const          TMap<KeyType, ValueType, Allocator>> = true;
template <typename KeyType, typename ValueType, typename Allocator> constexpr bool TIsTMap_V<      volatile TMap<KeyType, ValueType, Allocator>> = true;
template <typename KeyType, typename ValueType, typename Allocator> constexpr bool TIsTMap_V<const volatile TMap<KeyType, ValueType, Allocator>> = true;

template <typename T>
concept TIsTMap = TIsTMap_V<T>;

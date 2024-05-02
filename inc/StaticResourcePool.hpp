#pragma once

#include <array>

template <typename T, std::size_t capacity_>
class StaticResourcePool
{
    // private:
    //     using T = int;
    //     static constexpr std::size_t capacity_ = 3;

public:
    struct Node
    {
        friend StaticResourcePool;

    private:
        T item_;
        Node* next_;

        Node()
            : item_{}
            , next_{nullptr}
        {
        }

    public:
        T const& Item() const
        {
            return item_;
        }

        T& Item()
        {
            return item_;
        }
    };

private:
    Node* freePool_;
    Node* usedPool_;
    using StorageType = std::array<Node, capacity_>;
    StorageType storage_;

public:
    StaticResourcePool()
        : freePool_{nullptr}
        , usedPool_{nullptr}
        , storage_{}
    {
        for (std::size_t i = 0; i + 1 < capacity_; ++i)
        {
            storage_[i].next_ = &storage_[i + 1];
        }

        freePool_ = &storage_[0];
    }

    void Reset()
    {
        freePool_ = nullptr;
        usedPool_ = nullptr;
        new (&storage_) StorageType;

        for (std::size_t i = 1; i < capacity_; ++i)
        {
            storage_[i - 1].next_ = &storage_[i];
        }

        freePool_ = &storage_[0];
    }

    bool FreePoolEmpty() const
    {
        return freePool_ == nullptr;
    }

    bool UsedPoolEmpty() const
    {
        return usedPool_ == nullptr;
    }

    std::size_t FreePoolRemaining() const
    {
        std::size_t count = 0;
        auto p = freePool_;
        while (p != nullptr)
        {
            ++count;
            p = p->next_;
        }
        return count;
    }

    std::size_t UsedPoolRemaining() const
    {
        std::size_t count = 0;
        auto p = usedPool_;
        while (p != nullptr)
        {
            ++count;
            p = p->next_;
        }
        return count;
    }

    Node* TakeFree(void)
    {
        auto node = freePool_;
        if (node != nullptr)
        {
            freePool_ = node->next_;
            node->next_ = nullptr;
        }
        return node;
    }

    Node* TakeUsed(void)
    {
        auto node = usedPool_;
        if (node != nullptr)
        {
            usedPool_ = node->next_;
            node->next_ = nullptr;
        }
        return node;
    }

    void ReturnFree(Node* node)
    {
        // require pool_.begin() <= node && node < pool_.end();
        // require !InUnusedPool(node) && !InUsedPool(node)
        // require node != nullptr

        node->next_ = freePool_;
        freePool_ = node;
    }

    void ReturnUsed(Node* node)
    {
        // require pool_.begin() <= node && node < pool_.end();
        // require !InUnusedPool(node) && !InUsedPool(node)
        // require node != nullptr

        node->next_ = usedPool_;
        usedPool_ = node;
    }

    template <typename FuncType>
    bool ProcessFree(FuncType func)
    {
        auto node = TakeFree();
        bool poolNotEmpty = node != nullptr;
        if (poolNotEmpty)
        {
            func(node->item_);
            ReturnUsed(node);
        }
        return poolNotEmpty;
    }

    template <typename FuncType>
    bool ProcessUsed(FuncType func)
    {
        auto node = TakeUsed();
        bool poolNotEmpty = node != nullptr;
        if (poolNotEmpty)
        {
            func(node->item_);
            ReturnFree(node);
        }
        return poolNotEmpty;
    }
};

// Implementation is taken from https://github.com/CharlesFrasch/cppcon2023

#pragma once

#include <atomic>
#include <cassert>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>

template<typename T>
struct ValueSizeTraits
{
    using value_type = T;
    static std::size_t size(value_type const& value) { return sizeof(value_type); }
};


template<typename T, typename Alloc = std::allocator<T>>
class MyQueue : private Alloc
{
public:
    using value_type = T;
    using allocator_traits = std::allocator_traits<Alloc>;
    using size_type = typename allocator_traits::size_type;

    explicit MyQueue(size_type capacity, Alloc const& alloc = Alloc{})
        : Alloc{alloc}
        , capacity_{capacity}
        , ring_{allocator_traits::allocate(*this, capacity)}
    {}

    ~MyQueue() {
        allocator_traits::deallocate(*this, ring_, capacity());
    }


    auto size() const noexcept {
        auto pushCursor = pushCursor_.load(std::memory_order_relaxed);
        auto popCursor = popCursor_.load(std::memory_order_relaxed);

        assert(popCursor <= pushCursor);
        return pushCursor - popCursor;
    }

    auto empty() const noexcept { return size() == 0; }

    auto full() const noexcept { return size() == capacity(); }

    auto capacity() const noexcept { return capacity_; }


    class pusher_t
    {
    public:
        pusher_t() = default;
        explicit pusher_t(MyQueue* fifo, size_type cursor) noexcept : fifo_{fifo}, cursor_{cursor} {}

        pusher_t(pusher_t const&) = delete;
        pusher_t& operator=(pusher_t const&) = delete;

        pusher_t(pusher_t&& other) noexcept
            : fifo_{std::move(other.fifo_)}
            , cursor_{std::move(other.cursor_)} {
            other.release();
        }
        pusher_t& operator=(pusher_t&& other) noexcept {
            fifo_ = std::move(other.fifo_);
            cursor_ = std::move(other.cursor_);
            other.release();
            return *this;
        }

        ~pusher_t() {
            if (fifo_) {
                fifo_->pushCursor_.store(cursor_ + 1, std::memory_order_release);
            }
        }

        void release() noexcept { fifo_ = {}; }

        explicit operator bool() const noexcept { return fifo_; }

        value_type* get() noexcept { return fifo_->element(cursor_); }
        value_type const* get() const noexcept { return fifo_->element(cursor_); }

        value_type& operator*() noexcept { return *get(); }
        value_type const& operator*() const noexcept { return *get(); }

        value_type* operator->() noexcept { return get(); }
        value_type const* operator->() const noexcept { return get(); }

        pusher_t& operator=(value_type const& value) noexcept {
            std::memcpy(get(), std::addressof(value), ValueSizeTraits<value_type>::size(value));
            return *this;
        }

    private:
        MyQueue* fifo_{};
        size_type cursor_;
    };
    friend class pusher_t;

    pusher_t push() noexcept {
        auto pushCursor = pushCursor_.load(std::memory_order_relaxed);
        if (full(pushCursor, popCursorCached_)) {
            popCursorCached_ = popCursor_.load(std::memory_order_acquire);
            if (full(pushCursor, popCursorCached_)) {
                return pusher_t{};
            }
        }
        return pusher_t(this, pushCursor);
    }

    auto push(T const& value) noexcept {
        if (auto pusher = push(); pusher) {
            pusher = value;
            return true;
        }
        return false;
    }

    class popper_t
    {
    public:
        popper_t() = default;
        explicit popper_t(MyQueue* fifo, size_type cursor) noexcept : fifo_{fifo}, cursor_{cursor} {}

        popper_t(popper_t const&) = delete;
        popper_t& operator=(popper_t const&) = delete;

        popper_t(popper_t&& other) noexcept
            : fifo_{std::move(other.fifo_)}
            , cursor_{std::move(other.cursor_)} {
            other.release();
        }
        popper_t& operator=(popper_t&& other) noexcept {
            fifo_ = std::move(other.fifo_);
            cursor_ = std::move(other.cursor_);
            other.release();
            return *this;
        }

        ~popper_t() {
            if (fifo_) {
                fifo_->popCursor_.store(cursor_ + 1, std::memory_order_release);
            }
        }

        void release() noexcept { fifo_ = {}; }

        explicit operator bool() const noexcept { return fifo_; }

        value_type* get() noexcept { return fifo_->element(cursor_); }
        value_type const* get() const noexcept { return fifo_->element(cursor_); }

        value_type& operator*() noexcept { return *get(); }
        value_type const& operator*() const noexcept { return *get(); }

        value_type* operator->() noexcept { return get(); }
        value_type const* operator->() const noexcept { return get(); }

    private:
        MyQueue* fifo_{};
        size_type cursor_;
    };
    friend popper_t;

    auto pop() noexcept {
        auto popCursor = popCursor_.load(std::memory_order_relaxed);
        if (empty(pushCursorCached_, popCursor)) {
            pushCursorCached_ = pushCursor_.load(std::memory_order_acquire);
            if (empty(pushCursorCached_, popCursor)) {
                return popper_t{};
            }
        }
        return popper_t(this, popCursor);
    };

    auto pop(T& value) noexcept {
        if (auto popper = pop(); popper) {
            value = *popper;
            return true;
        }
        return false;
    }

private:
    auto full(size_type pushCursor, size_type popCursor) const noexcept {
        // assert(popCursor <= pushCursor);
        return (pushCursor - popCursor) == capacity();
    }
    static auto empty(size_type pushCursor, size_type popCursor) noexcept {
        return pushCursor == popCursor;
    }

    auto* element(size_type cursor) noexcept { return &ring_[cursor % capacity_]; }
    auto const* element(size_type cursor) const noexcept { return &ring_[cursor % capacity_]; }

private:
    size_type capacity_;
    T* ring_;

    using CursorType = std::atomic<size_type>;
    static_assert(CursorType::is_always_lock_free);

    static constexpr auto hardware_destructive_interference_size = size_type{64};

    alignas(hardware_destructive_interference_size) CursorType pushCursor_;

    alignas(hardware_destructive_interference_size) size_type popCursorCached_{};

    alignas(hardware_destructive_interference_size) CursorType popCursor_;

    alignas(hardware_destructive_interference_size) size_type pushCursorCached_{};

    char padding_[hardware_destructive_interference_size - sizeof(size_type)];
};

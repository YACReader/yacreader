#ifndef RELEASE_ACQUIRE_ATOMIC_H
#define RELEASE_ACQUIRE_ATOMIC_H

#include <atomic>

template<typename T>
class ReleaseAcquireAtomic
{
public:
    constexpr ReleaseAcquireAtomic(T desired) noexcept
        : value(desired) {}

    T operator=(T desired) noexcept
    {
        value.store(desired, std::memory_order_release);
        return desired;
    }

    operator T() const noexcept
    {
        return value.load(std::memory_order_acquire);
    }

private:
    std::atomic<T> value;
};

#endif // RELEASE_ACQUIRE_ATOMIC_H

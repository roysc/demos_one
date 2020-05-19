#pragma once

#include <functional>
#include <vector>

template<class... T>
struct observable
{
    using observer_function = std::function<void(T const&...)>;

    std::vector<observer_function> observers;

    auto hook(observer_function obs) { observers.emplace_back(obs); return observers.size() - 1; }
    auto hook(std::function<void()> obs) { hook([=](auto&&) { obs(); }); }

    struct _appender
    {
        observable<T...>& self;
        template <class F>
        auto& operator<<(F&& obs) { self.hook(obs); return *this; }
    };
    auto hooks() { return _appender{*this}; }

    template <class... Ts>
    void notify_all(Ts&&... a) { for (auto& obs: observers) { obs(std::forward<Ts>(a)...); } }
};

template <class T>
struct observable_value : public observable<T>
{
    T wrapped;

    template <class ...Args>
    observable_value(Args&&... args) : wrapped{args...} {}

    T& operator*() { return wrapped; }
    T const& operator*() const { return wrapped; }
    T const* operator->() const { return &wrapped; }

    void set(T that) { wrapped = that; notify_all(); }
    template <class... As>
    void emplace(As&&... args) { set(T{std::forward<As>(args)...}); }

    template <class F>
    void modify(F&& f) { f(wrapped); notify_all(); }

    void notify_all() { observable<T>::notify_all(wrapped); }
};

// Allows "block" syntax
#define Pz_observe(var_, ...) ((var_).hooks()) << [&](__VA_ARGS__)
#define Pz_observe_on(var_, chan_, ...) ((var_)._hook_##chan_##_.hooks()) << [&](__VA_ARGS__)
#define Pz_notify_observers(var_) (notify_observers(var_))

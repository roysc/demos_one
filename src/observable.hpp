#pragma once

#include <Rxt/util.hpp>
#include <functional>
#include <vector>

template<class T>
struct basic_observable
{
    using observer_function = T;

    std::vector<observer_function> observers;

    auto hook(observer_function obs) { observers.emplace_back(obs); return observers.size() - 1; }
    // auto hook(std::function<void()> obs) { hook([=](auto&&) { obs(); }); }

    struct _appender
    {
        basic_observable<T>& self;
        template <class F>
        auto& operator<<(F&& obs) { self.hook(obs); return *this; }
    };
    auto hooks() { return _appender{*this}; }

    template <class... Ts>
    void notify_all(Ts&&... a) { for (auto& obs: observers) { obs(std::forward<Ts>(a)...); } }
};

template <class...T>
using observable = basic_observable<std::function<void(T const&...)>>;

struct lazy_observable : public basic_observable<Rxt::lazy_action>
{
    void flush_all() { for (auto&& obs: observers) obs.flush(); }
};

template <class T>
struct observable_value : public basic_observable<T>
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

    void notify_all() { basic_observable<T>::notify_all(wrapped); }
};

// Allows "block" syntax
#define Pz_observe(var_, ...) ((var_).hooks()) << [&](__VA_ARGS__)
#define Pz_observe_on(var_, chan_, ...) ((var_)._hook_##chan_##_.hooks()) << [&](__VA_ARGS__)

#define Pz_notify(var_) ((var_)._hooks_.notify_all())

#define Pz_flush_on(var_, chan_) ((var_)._hook_##chan_##_.flush_all())
#define Pz_flush(var_) ((var_)._hooks_.flush_all())

// namespace tag
// {
// struct viewport {};
// struct cursor_motion {};
// struct cursor_selection {};
// }

// template <class... Tag>
// struct observer_router
// {
//     std::tuple<typename Tag::observer_type> subjects;
// };

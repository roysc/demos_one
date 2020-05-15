#pragma once

#include <functional>
#include <vector>
// #include <tuple>

template<class... T>
struct observable
{
    using observer_function = std::function<void(T const&...)>;

    std::vector<observer_function> observers;
    // using ArgsTuple = std::tuple<T...>;

    template <class F>
    auto hook(F&& obs) { observers.emplace_back(obs); return observers.size() - 1; }

    void notify_observers(T const&... a) { for (auto& obs: observers) { obs(a...); } }

    auto hooks()
    {
        struct Ret {
            observable<T...>& self;
            template <class F> operator+=(F&& obs) { self.hook(obs); }
        };
        return ret{*this};
    }
};

template <class T>
struct observable_value : public observable<T>
{
    // observable_value(T arg) : wrapped(arg) {}
    template <class ...Args>
    observable_value(Args&&... args) : wrapped{args...} {}

    T wrapped;
    void notify_observers() { observable<T>::notify_observers(wrapped); }
    void set(T that) { wrapped = that; notify_observers(); }
    template <class... As>
    void emplace(As&&... args) { set(T{std::forward<As>(args)...}); }
    template <class F>
    void modify(F&& f) { f(wrapped); notify_observers(); }

    T const& get() const { return wrapped; }
    T const* operator->() const { return &wrapped; }
};

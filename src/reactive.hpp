#pragma once

#include <functional>
#include <vector>
#include <map>
#include <optional>
#include <array>

namespace Rxt
{
template<class... Ts>
struct hooks
{
    using handler = std::function<void(Ts...)>;
    using index = std::size_t;

    std::vector<handler> observers;
    unsigned count = 0;

    index add(handler obs)
    {
        observers.push_back(obs);
        return observers.size() - 1;
    }

    void dispatch(Ts... t)
    {
        for (auto& obs: observers)
            obs(t...);
        count = 1;
    }

    int flush()
    {
        int ret = count;
        count = 0;
        return ret;
    }

    auto size() const { return observers.size(); }

    void operator()(Ts... t) { dispatch(t...); }

    template <class F>
    auto& operator+=(F&& h) { add(h); return *this; }
};

namespace _det
{
template <class T>
struct proxy_adder
{
    T& self;
    template <class F>
    auto& operator=(F&& h) { self.add(h); return self; }
};

template <class T>
proxy_adder(T&) -> proxy_adder<T>;
}

#define PZ_observe_capr(val_, ...) (::Rxt::_det::proxy_adder{(val_)}) = [&](__VA_ARGS__)
#define PZ_observe_capv(val_, ...) (::Rxt::_det::proxy_adder{(val_)}) = [=](__VA_ARGS__)
#define PZ_observe PZ_observe_capr
// #define PZ_observe(val_, ...) RXT_set_lambda(::Rxt::_det::proxy_adder{(val_)}, __VA_ARGS__)


template <class T, class Hook=hooks<>>
struct adapt_reactive : T
{
    using value_type = T;
    using super_type = T;
    using super_type::super_type;

    using hook_type = Hook;
    hook_type on_update;

    template <class...A>
    value_type& emplace(A&&...args)
    {
        this->super_type::operator=(value_type{std::forward<A>(args)...});
        // *this = value_type{std::forward<A>(args)...};
        on_update();
        return *this;
    }

    // static constexpr auto members()
    // {
    //     return std::array{ (&on_update) };
    // }
};

template <class Der>
struct reactive_base
{
    void do_update() { static_cast<Der&>(*this).on_update(); };
};

template <template <class...> class Crt, class Hook, class... T>
struct adapt_reactive_crt
    : Crt<adapt_reactive_crt<Crt, Hook, T...>, T...>
{
    using value_type = Crt<adapt_reactive_crt<Crt, Hook, T...>, T...>;
    using super_type = value_type;
    using super_type::super_type;

    using hook_type = Hook;
    hook_type on_update;

    template <class...A>
    value_type& emplace(A&&...args)
    {
        // this->super_type::operator=(value_type{std::forward<A>(args)...});
        new (this) value_type{std::forward<A>(args)...};
        on_update();
        return static_cast<value_type&>(*this);
    }
};

struct _hookless { void on_update() {} };

template <class Rh>
int flush_all(Rh& r)
{
    int ret = 0;
    for (auto& h: r) ret += h->flush();
    return ret;
}

struct toggle_hooks
{
    hooks<> on_enable, on_disable;
};

// Router hooks dispatch to active item
// Attaches per-item enable/disable hooks, which also dispatch back to router
template <class T, class H>
struct hook_router
{
    struct hook_type : H, toggle_hooks {};
    using map_type = std::map<T, hook_type>;

    hook_type _dispatch;
    map_type _map;
    std::optional<T> _switch;
    hooks<> on_update;

    hook_router()
    {
        for (auto m: H::members()) {
            auto mem = std::mem_fn(m);
            auto hook = [mem, this] {
                if (!_switch) return;
                auto it = _map.find(*_switch);
                if (it != end(_map))
                    mem(it->second)();
                on_update();
            };
            mem(_dispatch).add(hook);
        }
        PZ_observe(_dispatch.on_enable) { on_update(); };
        PZ_observe(_dispatch.on_disable) { on_update(); };
    }

    hook_type* operator->() { return &_dispatch; }
    auto& operator[](T k) { return _map[k]; }

    void insert(T k)
    {
        hook_type hook;
        PZ_observe(hook.on_enable) { _dispatch.on_enable(); };
        PZ_observe(hook.on_disable) { _dispatch.on_disable(); };
        _map.emplace(k, hook);
    }

    void disable()
    {
        if (_switch)
            _map[*_switch].on_disable();
    }

    void enable(T k)
    {
        disable();
        _switch.emplace(k);
        _map[*_switch].on_enable();
    }
};

// struct _empty { static constexpr auto members() {}};
// template <class H>
// using optional_hook = hook_router<_empty, H>;
// using hook_switch = hook_router<bool, H>;
}

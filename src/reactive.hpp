#pragma once

#include <functional>
#include <vector>
#include <map>
#include <optional>

namespace Rxt
{
inline namespace frp
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

    void operator()(Ts... t) { dispatch(t...); }
};

namespace _det
{
template <class T>
struct proxy_adder
{
    T& self;
    template <class F>
    auto& operator=(F&& h) { self.add(h); return *this; }
};

template <class T>
proxy_adder(T&) -> proxy_adder<T>;
}

#define PZ_observe_capr(val_, ...) (::Rxt::frp::_det::proxy_adder{(val_)}) = [&](__VA_ARGS__)
#define PZ_observe_capv(val_, ...) (::Rxt::frp::_det::proxy_adder{(val_)}) = [=](__VA_ARGS__)
#define PZ_observe PZ_observe_capr
// #define PZ_observe(val_, ...) RXT_set_lambda(::Rxt::frp::_det::proxy_adder{(val_)}, __VA_ARGS__)


template <class Reactive, class Hook=hooks<>>
struct adapt_reactive : Reactive
{
    using reactive_base = Reactive;
    using reactive_base::reactive_base;
    using hook_type = Hook;

    hook_type on_update;

    auto& emplace(reactive_base that)
    {
        this->reactive_base::operator=(that);
        on_update();
        return *this;
    }

    reactive_base& operator*() { return *this; }
};

template <template <class...> class Reactive, class Hook, class... T>
struct adapt_reactive_crt
    : adapt_reactive<Reactive<adapt_reactive_crt<Reactive, Hook, T...>, T...>, Hook>
{
    using super_type = adapt_reactive<Reactive<adapt_reactive_crt<Reactive, Hook, T...>, T...>, Hook>;
    using super_type::super_type;
};

struct no_hook { void on_update() {} };

template <class Rh>
int flush_all(Rh& r)
// int flush_all(range_of<hooks<>&> auto r)
{
    int ret = 0;
    for (auto& h: r) ret += h->flush();
    return ret;
}


// Router hooks dispatch to active item
// Attaches per-item enable/disable hooks, which also dispatch back to router
template <class T, class H>
struct hook_router
{
    struct hook_type : H { hooks<> on_enable, on_disable; };

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
        _switch = k;
        _map[*_switch].on_enable();
    }
};

}
}

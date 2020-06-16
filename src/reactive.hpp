#pragma once

#include "observable.hpp"
#include <functional>
#include <vector>

namespace //obs
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

template <class Reactive>
struct adapt_reactive : Reactive
{
    using reactive_base = Reactive;
    using reactive_base::reactive_base;

    hooks<> on_update;

    auto& emplace(reactive_base that)
    {
        this->reactive_base::operator=(that);
        on_update();
        return *this;
    }

    reactive_base& operator*() { return *this; }
};

template <template <class...> class Reactive, class... T>
struct adapt_reactive_crt
    : adapt_reactive<Reactive<adapt_reactive_crt<Reactive, T...>, T...>>
{
    using super_type =
        adapt_reactive<Reactive<adapt_reactive_crt<Reactive, T...>, T...>>;
    // using super_type::super_type;
};

template <class Rh>
int flush_all(Rh& r)
// int flush_all(range_of<hooks<>&> auto r)
{
    int ret = 0;
    for (auto& h: r) ret += h->flush();
    return ret;
}
}

#pragma once

#include <functional>
#include <vector>

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

template <template <class...> class Reactive, class... T>
struct hooked
    : Reactive<hooked<Reactive, T...>, T...>
{
    using base_type = Reactive<hooked, T...>;
    using base_type::base_type;
    hooks<> on_update;
};

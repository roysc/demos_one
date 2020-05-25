#pragma once

#include <Rxt/util.hpp>
#include <Rxt/meta.hpp>

#include <functional>
#include <vector>
#include <tuple>

template <class Tag>
struct subject
{
    using tag = Tag;
    using index = std::size_t;
    using handler = std::function<void(tag)>;

    virtual index add(handler) = 0;
    virtual void dispatch(tag = {}) = 0;
    virtual int flush() = 0;

    virtual ~subject() {}
};

namespace _det
{
template <class Sub>
struct proxy_appender
{
    Sub& self;
    template <class F>
    auto& operator<<(F&& h) { self.add(h); return *this; }
};

template <class Sub>
proxy_appender(Sub&) -> proxy_appender<Sub>;
}

#define Pz_observe(val_) (_det::proxy_appender{(val_)}) << [&](auto)

template<class Tag>
struct eager_observable : public subject<Tag>
{
    using index = typename subject<Tag>::index;
    using handler = typename subject<Tag>::handler;

    std::vector<handler> observers;
    unsigned count = 0;

    index add(handler obs) override
    {
        observers.push_back(obs);
        return observers.size() - 1;
    }

    void dispatch(Tag t) override
    {
        for (auto& obs: observers) { obs(t); }
        count = 1;
    }

    int flush() override { int ret = count; count = 0; return ret; }

    void operator()() { dispatch(Tag{}); }
};

template<class Tag>
struct lazy_observable : public subject<Tag>
{
    using index = typename subject<Tag>::index;
    using handler = typename subject<Tag>::handler;
    using lazy_handler = Rxt::lazy_action;

    std::vector<lazy_handler> observers;

    index add(handler obs) override
    {
        observers.emplace_back([obs] { obs(Tag{}); });
        return observers.size() - 1;
    }

    void dispatch(Tag) override { for (auto& obs: observers) { obs(); } }

    int flush() override
    {
        int ret = 0;
        for (auto& obs: observers) { ret += obs.flush(); }
        return ret;
    }

    void operator()() { dispatch(Tag{}); }
};

template <class Tag>
using observable = eager_observable<Tag>;

template <class... Ts>
struct observer_router
{
    std::tuple<subject<Ts>*...> subjects;

    template <class Tag>
    auto& _get() { return std::get<subject<Tag>*>(subjects); }

    template <class Tag>
    auto& get_subject(Tag t = Tag{}) { return *_get<Tag>(); }

    template <class Tag>
    void add_subject(subject<Tag>& sub) { _get<Tag>() = &sub; }

    // void dispatch() { (get_subject<Ts>()->dispatch(Ts{}), ...); }

    int flush()
    {
        auto safe_flush = [](auto* p) { return p ? p->flush() : 0; };
        return (safe_flush(_get<Ts>()) + ...);
    }
};

// namespace _det
// {
template <class... Ts>
using observable_tags = Rxt::type_tuple<typename Ts::tag...>;

template <class TT>
using observable_tags_for_t = Rxt::tuple_apply_t<observable_tags, TT>;

template <class TT>
using router_for_t = Rxt::tuple_apply_t<observer_router, TT>;
// }

template <class... Obs>
struct multi_observable
{
    using observables_tuple = Rxt::type_tuple<Obs...>;
    using tags_tuple = observable_tags<Obs...>;
    std::tuple<Obs...> _data;

    template <class Tag>
    auto& get(Tag = {}) { return std::get<Rxt::tuple_index_of_v<Tag, tags_tuple>>(_data); }

    template <class R>
    void add_to_router(R& rout)
    {
        (rout.template add_subject<typename Obs::tag>(get<typename Obs::tag>()), ...);
    }

    int flush() { return (std::get<Obs>(_data).flush() + ...); }
};

template <class T, class Obs>
struct observable_proxy : T
{
    using proxy_type = T;
    using observable_type = Obs;

    observable_type _subject;

    using proxy_type::proxy_type;

    template <class Tag>
    auto& on(Tag t)
    {
        return _subject.get(t);
    }

    observable_type& subject() { return _subject; }
};

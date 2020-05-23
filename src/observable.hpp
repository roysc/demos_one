#pragma once

#include <Rxt/util.hpp>
#include <functional>
#include <vector>
#include <tuple>

template <class Tag>
struct subject
{
    using tag = Tag;
    using index = std::size_t;
    using handler = std::function<void(tag)>;

    virtual index hook(handler) = 0;
    virtual void notify(tag) = 0;
    virtual int flush() = 0;
};

namespace _det
{
template <class Sub>
struct hook_appender
{
    Sub* self;
    template <class F>
    auto& operator<<(F&& h) { self->hook(h); return *this; }
};

template <class Sub>
auto hooks(Sub& s) { return hook_appender<Sub>{&s}; }
}

template<class Tag>
struct eager_observable : public subject<Tag>
{
    using arg_type = Tag;
    using index = typename subject<Tag>::index;
    using handler = typename subject<Tag>::handler;

    std::vector<handler> observers;
    unsigned count = 0;

    index hook(handler obs) override
    {
        observers.push_back(obs);
        return observers.size() - 1;
    }

    void notify(Tag t) override
    {
        for (auto& obs: observers) { obs(t); }
        count = 1;
    }

    void operator()() { notify(Tag{}); }

    int flush() override { int ret = count; count = 0; return ret; }
};

template<class Tag>
struct lazy_observable : subject<Tag>
{
    using arg_type = Tag;
    using index = typename subject<Tag>::index;
    using handler = typename subject<Tag>::handler;
    using lazy_handler = Rxt::lazy_action;

    std::vector<lazy_handler> observers;

    index hook(handler obs) override
    {
        observers.emplace_back([obs] { obs(Tag{}); });
        return observers.size() - 1;
    }

    void notify(Tag) override { for (auto& obs: observers) { obs(); } }

    void operator()() { notify(Tag{}); }

    int flush() override
    {
        int ret = 0;
        for (auto& obs: observers) { ret += obs.flush(); }
        return ret;
    }
};

template <class Tag>
using observable = eager_observable<Tag>;

template <class Tag>
struct subject_ref
{
    using self_ptr = subject<Tag>*;
    self_ptr self{};

    auto& operator=(self_ptr p) { self = p; }
    void operator()() { if (!self) throw nullptr; self->notify(Tag{}); }
    // auto hooks() { if (!self) throw nullptr; return self->hooks; }
};

template <class... Ts>
struct observer_router
{
    std::tuple<subject<Ts>*...> subjects;

    template <class Tag>
    auto& get_subject(Tag t) { return *std::get<subject<Tag>*>(subjects); }

    template <class Sub>
    void add_subject(Sub& subj)
    {
        using Arg = typename Sub::arg_type;
        std::get<subject<Arg>*>(subjects) = &subj;
    }

    int flush()
    {
        auto safe_flush = [](auto* p) { return p ? p->flush() : 0; };
        return (safe_flush(std::get<subject<Ts>*>(subjects)) + ...);
    }
};


// #define Pz_observe(obr_, tag_) (_det::hooks((obr_).get_subject(tag_{}))) << [&](tag_)
#define Pz_observe(val_) (_det::hooks((val_))) << [&](auto)
#define Pz_notify(obr_, tag_) ((obr_).tag_ref(tag_{})())
#define Pz_flush(var_, tag_) ((var_).get_subject(tag_{}).flush())
#define Pz_flush_all(obr_) ((obr_).flush())

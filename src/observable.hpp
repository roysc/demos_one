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
    using handler = std::function<void(Tag)>;

    virtual void notify_all(Tag) = 0;
    virtual index hook(handler) = 0;
};

namespace _det
{
template <class Sub>
struct hook_appender
{
    Sub& self;
    template <class F>
    auto& operator<<(F&& h) { self->hook(h); return *this; }
};

template <class Sub>
auto hooks(Sub& s) { return hook_appender<Sub>{s}; }
}

template<class Tag>
struct basic_subject : public subject<Tag>
{
    using index = typename subject<Tag>::index;
    using handler = typename subject<Tag>::handler;

    std::vector<handler> observers;

    index hook(handler obs) override
    {
        observers.push_back(obs);
        return observers.size() - 1;
    }

    void notify_all(Tag t) override { for (auto& obs: observers) { obs(t); } }
    void operator()() { notify_all(Tag{}); }
};

template<class Tag>
struct lazy_subject : subject<Tag>
{
    using index = typename subject<Tag>::index;
    using handler = typename subject<Tag>::handler;
    using lazy_handler = Rxt::lazy_action;

    std::vector<lazy_handler> observers;

    index hook(handler obs) override
    {
        observers.push_back(obs);
        return observers.size() - 1;
    }

    void notify_all(Tag t) override { for (auto& obs: observers) { obs(t); } }
    void operator()() { notify_all(Tag{}); }

    auto flush_all(Tag t) override
    {
        unsigned ret {};
        for (auto& obs: observers) { ret += obs.flush(); }
        return ret;
    }
};

template <class Tag>
using observable = basic_subject<Tag>;

template <class Tag>
struct _subject_ref
{
    using self_ptr = subject<Tag>*;
    self_ptr self{};

    auto& operator=(self_ptr p) { self = p; }
    void operator()() { if (!self) throw nullptr; self->notify_all(Tag{}); }
    // auto hooks() { if (!self) throw nullptr; return self->hooks; }
};

template <class Tag>
using subject_ref = _subject_ref<Tag>;

template <class Tag> using eager_observable = observable<Tag>;

template <class... Tags>
struct observer_router
{
    std::tuple<subject<Tags>*...> subjects;

    template <class Tag>
    auto& get_subject(Tag t) { return std::get<subject<Tag>*>(subjects); }

    template <class Tag>
    void set_subject(Tag, subject<Tag>& subj) { std::get<subject<Tag>*>(subjects) = &subj; }

    template <class Tag>
    auto tag_ref(Tag t) { return subject_ref<Tag>{get_subject(t)}; }
};


#define Pz_observe(obr_, tag_) (_det::hooks((obr_).get_subject(tag_{}))) << [&](tag_)
#define Pz_notify(obr_, tag_) ((obr_).tag_ref(tag_{})())

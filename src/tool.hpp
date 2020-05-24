#pragma once
#include "mouse.hpp"
#include "observable.hpp"

namespace _det
{
template <class Rout>
struct proxy_multi_appender
{
    Rout& router;
    typename Rout::index const index;

    template <class Tag>
    auto& operator()(Tag t)
    {
        return router.get_subject(t, index);
    }
};

template<class Rout>
proxy_multi_appender(Rout&...) -> proxy_multi_appender<Rout>;
}

template <class... Tags>
struct swappable_tool : mouse_tool
{
    using index = std::size_t;
    using subject_tuple = std::tuple<
        eager_observable<tags::activate_tag>,
        eager_observable<tags::deactivate_tag>,
        eager_observable<Tags>...>;

    std::vector<mouse_tool*> tools;
    std::vector<subject_tuple> subjects;
    subject_tuple shared;

    index current_tool = -1;
    std::map<mouse_tool*, index> _pindex;

    mouse_tool* current()
    {
        if (current_tool == -1)
            return nullptr;
        return tools.at(current_tool);
    }

    void set_current(index i)
    {
        if (current_tool == i) return;
        deactivate(current_tool);
        current_tool = i;
        activate(current_tool);
    }

    void deactivate(index i) { get_subject(tags::deactivate, i)(); }
    void activate(index i) { get_subject(tags::activate, i)(); }

    void set_current(mouse_tool* p) { set_current(_pindex.at(p)); }

    subject_tuple& get_subjects(index ix)
    {
        return subjects.at(ix);
    }

    template <class Tag>
    auto& _get(subject_tuple& tup)
    {
        return std::get<eager_observable<Tag>>(tup);
    }

    template <class Tag>
    auto& get_subject(Tag t, index ix)
    {
        return _get<Tag>(subjects.at(ix));
    }

    template <class Tag>
    auto& get_shared(Tag)
    {
        return _get<Tag>(shared);
    }

    auto add_tool(mouse_tool* t, bool also_set = false)
    {
        auto ix = tools.size();
        tools.push_back(t);
        _pindex[t] = ix;
        subjects.emplace_back();
        if (also_set)
            current_tool = ix;
        return _det::proxy_multi_appender{*this, ix};
    }

    template <class Tag>
    void dispatch(Tag t)
    {
        _get<Tag>(shared).notify(t);
        get_subject(t, current_tool).notify(t);
    }

    int flush()
    {
        int sum = 0;
        sum += (_get<Tags>(shared).flush() + ...);
        for (auto& sub: subjects) {
            sum += (std::get<eager_observable<Tags>>(sub).flush() + ...);
        }
        return sum;
    }

    void mouse_down(int i) override
    {
        // if constexpr (_debug)
        Rxt::print("mouse_down({})\n", i);
        if (current()) current()->mouse_down(i);
    }

    void mouse_up(int i) override
    {
        Rxt::print("mouse_up({})\n", i);
        if (current()) current()->mouse_up(i);
    }
};

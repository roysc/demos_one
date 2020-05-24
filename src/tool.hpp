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
    using subject_tuple = std::tuple<eager_observable<tags::reset>,
                                     eager_observable<Tags>...>;

    std::vector<mouse_tool*> tools;
    std::vector<subject_tuple> subjects;

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
        get_subject(tags::reset(), current_tool)();
        current_tool = i;
    }

    void set_current(mouse_tool* p) { set_current(_pindex.at(p)); }

    subject_tuple& get_subjects(index ix)
    {
        return subjects.at(ix);
    }

    template <class Tag>
    auto& get_subject(Tag t, index ix)
    {
        return std::get<eager_observable<Tag>>(subjects.at(ix));
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
        get_subject(t, current_tool).notify(t);
    }

    int flush()
    {
        int sum = 0;
        for (auto& sub: subjects) {
            sum += (std::get<eager_observable<Tags>>(sub).flush() + ...);
        }
        return sum;
    }

    // operator bool() const { return current_tool != -1; }

    void mouse_down(int i) override { if (current()) current()->mouse_down(i); }
    void mouse_up(int i) override { if (current()) current()->mouse_up(i); }
};

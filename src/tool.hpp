#pragma once

#include "mouse.hpp"
#include "observable.hpp"

#include <Rxt/meta.hpp>
#include <tuple>

template <class ExtraTags>
struct swappable_tool : mouse_tool
{
    using index = std::size_t;

    using builtin_tags = Rxt::type_tuple<
        tags::activate_tag,
        tags::deactivate_tag
        >;
    using observable_tags = Rxt::tuple_concat_t<ExtraTags, builtin_tags>;
    using router_type = Rxt::tuple_apply_t<observer_router, observable_tags>;

    std::vector<router_type> routers;
    router_type always;

    std::vector<mouse_tool*> tools;
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
        if (current()) deactivate(current_tool);
        current_tool = i;
        activate(current_tool);
    }

    void set_current(mouse_tool* p) { set_current(_pindex.at(p)); }

    void deactivate(index i) { get_router(i).get_subject(tags::deactivate).dispatch({}); }
    void activate(index i) { get_router(i).get_subject(tags::activate).dispatch({}); }

    template <class Tag>
    subject<Tag>& get_shared(Tag t)
    {
        return always.get_subject(t);
    }

    template <class Tag>
    auto& on(Tag t) { return get_shared(t); }

    template <class S>
    auto add_shared_subjects(S& sub)
    {
        sub.add_to_router(always);
    }

    router_type& get_router(index ix)
    {
        return routers.at(ix);
    }

    template <class T>
    auto add_tool(T& tool, bool also_set = false)
    {
        auto* toolptr = &tool;
        auto ix = tools.size();
        tools.push_back(toolptr);
        _pindex[toolptr] = ix;

        auto& rout = routers.emplace_back();
        tool.subject().add_to_router(rout);
        return ix;
    }

    template <class Tag>
    void dispatch(Tag t)
    {
        get_shared(t).dispatch(t);
        get_router(current_tool).get_subject(t).dispatch(t);
    }

    int flush()
    {
        int sum = 0;
        sum += always.flush();
        for (auto& rout: routers) {
            sum += rout.flush();
        }
        return sum;
    }

    void mouse_down(mouse_button i) override
    {
        // if constexpr (_debug)
        // Rxt::print("mouse_down({})\n", i);
        if (current()) current()->mouse_down(i);
    }

    void mouse_up(mouse_button i) override
    {
        // Rxt::print("mouse_up({})\n", i);
        if (current()) current()->mouse_up(i);
    }
};

#pragma once

#include "map.hpp"
#include <Rxt/io.hpp>

#include <list>
#include <memory>
#include <string>
#include <format>

// Fat pointer to a single cell
template <class S>
struct stage_cell
{
    // grid / float?
    // variant<ipos2, fpos3> r;
    using vec_type = typename S::position_type;
    S* env;
    vec_type _pos;

    stage_cell(S& e, vec_type v)
        : env(&e)
        , _pos(v)
    {}

    auto& stage() { return *env; }
    auto position() const { return _pos; }
    auto value() const { return env->grid().at(_pos); }
    // auto substage() {}
};

template <class Vec, class S>
Vec offset(stage_cell<S> const& c)
{
    auto elev = float(c.value()) / 0xFF; // todo
    return Vec(c.position(), elev) + Vec(.5, .5, 0);
}

template <class P>
struct cell_path
{
    using list = std::list<P>;
    list path;

    void append(cell_path rest) { path.insert(rest.path.begin(), rest.path.end()); }

    void append(P part) { path.push_back(part); }
};

// Recursive data-owning stage wrapper (tree node)
template <class Stage>
struct deep_stage : public Stage
{
    using super_type = Stage;
    using position_type = typename Stage::position_type;

    using pointer = std::unique_ptr<deep_stage>; // tradeoffs v T*?
    using depth_t = unsigned char;
    using cell_type = stage_cell<deep_stage>;
    using stage_path = cell_path<position_type>;

    // contains our position in the superstage
    struct superstage_cell
    {
        deep_stage* stage = nullptr;
        position_type pos{0};
        operator bool() const { return stage; }
    };

    dense_grid<pointer> _substages;
    superstage_cell _address;

    template <class U>
    deep_stage(U& uni, superstage_cell super = {})
        : super_type{uni}
        , _substages{this->size()}
        , _address{super}
    {}

    // return full cell path to this stage
    stage_path get_path()
    {
        stage_path ret;
        if (_address) {
            ret = _address.stage->get_path();
            ret.append(_address.pos);
        }
        return ret;
    }

    auto depth() const { return get_path().size(); }

    deep_stage* get_substage(position_type pos, bool create = true)
    {
        auto& ptr = _substages.at(pos);
        if (!ptr && create) {
            ptr.reset(new deep_stage(this->space(), {.stage = this, .pos = pos}));
        }
        return ptr.get();
    }
};

template <class S>
struct std::formatter<stage_cell<S>>
{
    template <class FC>
    auto format(stage_cell<S> const& cell, FC& ctx) const
    {
        return std::format_to(ctx.out(), "cell({1}, env={0})", (std::size_t)cell.env, cell.r);
    }
};

template <class P>
struct std::formatter<cell_path<P>>
{
    auto parse(format_parse_context& c) { return c.begin(); }

    template <class FC>
    auto format(cell_path<P> const& cp, FC& ctx) const
    {
        unsigned i = 0;
        auto out = ctx.out();
        for (auto p : cp.path) {
            if (i++)
                out = format_to(out, "/");
            out = format_to(out, "{}:{}", p.x, p.y);
        }
        return out;
    }
};

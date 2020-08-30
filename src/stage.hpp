#pragma once
#include "map.hpp"
#include <Rxt/io.hpp>
#include <memory>
#include <list>


// Fat pointer to a single cell
template <class S>
struct stage_cell
{
    // grid / float?
    // variant<ipos2, fpos3> r;
    using vec_type = typename S::position_type;
    S* env;
    vec_type _pos;

    stage_cell(S& e, vec_type v) : env(&e), _pos(v) {}

    auto& stage() { return *env; }
    auto position() const { return _pos; }
    auto value() const { return env->grid().at(_pos); }

    template <class Vec>
    Vec offset() const
    {
        auto elev = float(value()) / 0xFF; // todo
        return Vec(_pos, elev) + Vec(.5,.5,0);
    }
    // auto substage() {}
};


// Recursive data-owning stage wrapper (tree node)
template <class Stage>
struct deep_stage
    : Stage
{
    using super_type = Stage;
    using position_type = Stage::position_type;

    using pointer = std::unique_ptr<deep_stage>; // tradeoffs v T*?
    using depth_t = unsigned char;
    using cell_type = stage_cell<deep_stage>;

    dense_grid<pointer> _substages;

    // contains our position in the superstage
    struct superstage_cell
    {
        deep_stage* stage = nullptr;
        position_type pos{0};
        operator bool() const { return stage; }
    };
    superstage_cell _address;

    template <class U>
    deep_stage(U& uni, superstage_cell super = {})
        : super_type{uni}
        , _substages{this->size()}
        , _address{super}
    {}

    using cell_path = std::list<position_type>;

    // return full cell path to this stage
    cell_path get_path()
    {
        cell_path ret;
        if (_address) {
            ret = _address->stage().get_path();
            ret.push_back(_address->position());
        }
        return ret;
    }
    auto depth() const { return get_path().size(); }

    deep_stage* get_substage(position_type pos, bool create = true)
    {
        auto& ptr = _substages.at(pos);
        if (!ptr && create) {
            ptr.reset(new deep_stage(this->universe(), {.stage = this, .pos = pos}));
        }
        return ptr.get();
    }

};

// template <class Part>
// struct cell_path
// {
//     using list = std::list<Path>;
//     list path;

//     auto& operator +=(cell_path rest)
//     {
//         path.insert(rest.path.begin(), rest.path.end());
//     }

//     auto& operator +=(Part part)
//     {
//         path.push_back(part);
//     }
// };

// template <class Pt>
// struct fmt::formatter<cell_path<Pt>>
// {
//     template <class FC>
//     auto format(stage_cell<S> const& cell, FC& ctx)
//     {
//         return fmt::format_to(ctx.out(), "cell({1}, env={0})", (std::size_t)cell.env, cell.r);
//     }
// };

template <class S>
struct fmt::formatter<stage_cell<S>>
{
    template <class FC>
    auto format(stage_cell<S> const& cell, FC& ctx)
    {
        return fmt::format_to(ctx.out(), "cell({1}, env={0})", (std::size_t)cell.env, cell.r);
    }
};

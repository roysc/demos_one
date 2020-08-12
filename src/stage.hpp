#pragma once
#include <memory>
#include <list>

// #include <fmt/fmt.hpp>
#include <Rxt/io.hpp>


// Fat pointer to a single cell
template <class S>
struct stage_cell
{
    // grid / float?
    // variant<ipos2, fpos3> r;
    using vec_type = typename S::position_type;
    S* env;
    vec_type r;

    stage_cell(S& e, vec_type v) : env(&e), r(v) {}

    auto& stage() { return *env; }
    auto position() const { return r; }
    auto value() const { return env->grid().at(r); }

    template <class Vec>
    Vec offset() const
    {
        auto elev = float(value()) / 0xFF; // todo
        return Vec(r, elev) + Vec(.5,.5,0);
    }
    // auto substage() {}
};


// Recursive wrapper for stages
template <class Stage>
struct deep_stage;

template <class Stage>
struct deep_stage
    : Stage
{
    using super_type = Stage;
    using position_type = Stage::position_type;

    using pointer = std::unique_ptr<deep_stage>;
    using depth_t = unsigned char;
    using cell_type = stage_cell<deep_stage>;

    dense_grid<pointer> _substages;
    // deep_stage* _superstage;

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

    using address_path = std::list<position_type>;

    // return full cell path to this stage
    address_path get_path(position_type pos)
    {
        address_path ret;
        if (_address) {
            ret = _address->stage().get_path();
            ret.push_back(_address->position());
        }
        ret.push_back(pos);
        return ret;
    }

    auto get_substage(position_type pos, bool create = true)
    {
        auto& ptr = _substages.at(pos);
        if (!ptr && create) {
            ptr.emplace(new deep_stage(this->universe(), {.stage = this, .pos = pos}));
        }
        return ptr.get();
    }
};

// template <class Part>
// struct address_path
// {
//     using list = std::list<Path>;
//     list path;

//     auto& operator +=(address_path rest)
//     {
//         path.insert(rest.path.begin(), rest.path.end());
//     }

//     auto& operator +=(Part part)
//     {
//         path.push_back(part);
//     }
// };

// template <class Pt>
// struct fmt::formatter<address_path<Pt>>
// {
//     template <class PC>
//     constexpr auto parse(PC& ctx)
//     {
//         return ctx.begin();
//     }

//     template <class FC>
//     auto format(stage_cell<S> const& cell, FC& ctx)
//     {
//         return fmt::format_to(ctx.out(), "cell({1}, env={0})", (std::size_t)cell.env, cell.r);
//     }
// };

template <class S>
struct fmt::formatter<stage_cell<S>>
{
    template <class PC>
    constexpr auto parse(PC& ctx)
    {
        return ctx.begin();
    }

    template <class FC>
    auto format(stage_cell<S> const& cell, FC& ctx)
    {
        return fmt::format_to(ctx.out(), "cell({1}, env={0})", (std::size_t)cell.env, cell.r);
    }
};

#pragma once
#include <bitset>

// split by bit
struct bsplit
{
    static constexpr auto depth = 16;
    using key_type = std::bitset<depth>;

    static bool compare(key_type a, key_type b)
    {
        return a.to_ulong() < b.to_ulong();
    }
    using map_type = std::set<key_type, decltype(&compare)>;    
};
using bix = bsplit::key_type;

struct mineral_species { bix _m; };
struct biota_species { bix _m; };
struct plasma_species { bix _m; };
struct fluid_species { bix _m; };
struct gas_species { bix _m; };
struct vac {};

struct mineral {
    mineral_species species;
};
struct fluid {
    fluid_species species;
};
struct gas {
    gas_species species;
};
struct plasma {
    plasma_species species;
};
struct biota {
    biota_species species;
};

#pragma once

template <unsigned ix, class Vec>
Vec invert(Vec v) { v[ix] = -v[ix]; return v; }

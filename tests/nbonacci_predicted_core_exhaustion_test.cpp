#define main nbonacci_arithmetic_hull_program_main
#include "../app/nbonacci_arithmetic_hull.cpp"
#undef main

#include <cstdio>

int main() {
    Options options;
    options.coefficient_bound = 1;
    options.dump_core_exhaustion = true;
    bool ok = true;
    for (std::size_t n = 3; n <= 7; ++n)
        ok = check(n, options) && ok;
    std::printf("predicted-core exhaustion audit: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

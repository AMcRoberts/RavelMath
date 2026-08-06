#include <iostream>
#include "ravel/proof/universal_dominance_campaign.hpp"

int main() {
    using namespace ravel::proof::universal_dominance;
    std::cout << render_campaign_report(build_universal_nbonacci_dominance_map());
    return 0;
}

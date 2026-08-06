#include <cassert>
#include <string>
#include "ravel/proof/typed_theorem_application.hpp"

int main() {
    using namespace ravel::proof::truth;
    TypedTheoremApplicator applicator;
    ApplicationRequest missing{"charpoly-to-fast-power-reduction", {}};
    auto unresolved = applicator.apply(missing);
    assert(!unresolved.closed);
    assert(!unresolved.missing.empty());

    ApplicationRequest request;
    request.rule_id = "charpoly-to-fast-power-reduction";
    request.hypotheses.emplace("charpoly_identity", TypedHypothesis{
        "charpoly_identity", "hchar", "A.charpoly = nbonacciCharpoly n"});
    auto closed = applicator.apply(request);
    assert(closed.closed);
    assert(closed.lean_source.find("truth_machine_fast_power_reduction") != std::string::npos);
    assert(closed.lean_source.find("nbonacci_geomSum_of_charpoly A n hchar") != std::string::npos);
    assert(closed.lean_source.find("sorry") == std::string::npos);
    return 0;
}

#include <cassert>
#include <iostream>
#include <string>

#include "ravel/proof/universal_dominance_campaign.hpp"

int main() {
    using namespace ravel::proof::universal_dominance;

    const auto map = build_universal_nbonacci_dominance_map();
    assert(map.theorem_id == "nbonacci.boundary_dominance.universal");
    assert(map.obligations.size() >= 8);
    assert(!map.active_seams.empty());

    bool found_shell = false;
    bool found_spectral = false;
    bool found_internal_core = false;
    bool found_literal_core = false;
    for (const auto& obligation : map.obligations) {
        if (obligation.id == "carry.parametric_shell_partition") {
            found_shell = true;
            assert(obligation.status == ObligationStatus::ExactCertificate);
        }
        if (obligation.id == "quotient.core_spectral_maximality") {
            found_spectral = true;
            assert(obligation.status == ObligationStatus::ExactCertificate);
        }
        if (obligation.id == "core.universal_identification") {
            found_internal_core = true;
            assert(obligation.status == ObligationStatus::ExactCertificate);
        }
        if (obligation.id == "quotient.core_spectral_maximality") {
            found_literal_core = true;
        }
    }
    assert(found_shell && found_spectral && found_internal_core &&
           found_literal_core);

    ShellReturnCertificate cert;
    cert.dimension = 3;
    cert.shell_radius = 2;
    cert.shell_states = {{2,0,0}, {0,2,0}, {0,0,2}};
    cert.edges = {{0,1}, {1,2}};
    cert.rank = {0,1,2};
    auto validation = validate_shell_return_certificate(cert);
    assert(validation.dimensions_consistent);
    assert(validation.endpoints_valid);
    assert(validation.rank_total);
    assert(validation.rank_strict);
    assert(!validation.certificate_closed); // last state has no return edge

    cert.edges.push_back({2,0});
    validation = validate_shell_return_certificate(cert);
    assert(validation.certificate_closed);
    assert(!validation.rank_strict); // a closed finite strict rank is impossible

    std::cout << render_campaign_report(map);
    return 0;
}

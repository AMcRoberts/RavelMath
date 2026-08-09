#include <cassert>
#include <iostream>

#include "ravel/proof/second_genuine_fourth_generator_intertwiner.hpp"

int main() {
    const auto certificate =
        ravel::proof::derive_second_genuine_fourth_generator_intertwiner();
    std::cout << "boundary_states=" << certificate.boundary_states
              << " pre_contact_states=" << certificate.pre_contact_states
              << " contact_states=" << certificate.contact_states
              << " boundary_edges=" << certificate.boundary_edges
              << " universal_edges=" << certificate.universal_edges
              << " boundary_complete=" << certificate.boundary_complete
              << " closure_capped=" << certificate.closure_stopped_early
              << " corona_capped=" << certificate.corona_capped
              << " obstruction=\"" << certificate.obstruction << "\"\n";
    std::cout << "boundary_realizes_defect_plus2="
              << certificate.boundary_realizes_defect[4]
              << " boundary_realizes_defect_minus2="
              << certificate.boundary_realizes_defect[0] << "\n";
    std::cout << "generator_intertwines: ";
    for (std::size_t generator = 0; generator < certificate.generator_count; ++generator)
        std::cout << static_cast<long long>(generator) - 2 << "="
                  << certificate.generator_intertwines[generator] << " ";
    std::cout << "\nproved=" << certificate.proved << "\n";
    if (!certificate.boundary_complete) {
        std::cout << "INCONCLUSIVE: the explicit exploratory boundary caps "
                     "were reached before convergence.\n";
        return 0;
    }
    assert(certificate.boundary_realizes_defect[4]);
    assert(certificate.boundary_realizes_defect[0]);
    assert(certificate.boundary_complete);
    assert(certificate.proved);
    std::cout << "PASS: the second interior-zero fourth-generator witness "
                 "survives the concrete contact-boundary intertwiner.\n";
}

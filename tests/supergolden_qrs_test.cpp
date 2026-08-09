#include <cassert>
#include <fstream>
#include <iostream>
#include "ravel/proof/supergolden_qrs_closure.hpp"
int main(){
 auto c=ravel::proof::derive_supergolden_qrs_closure();
 assert(c.proved);
 assert((c.greedy_digits == std::vector<long long>{1,0,1}));
 assert(c.alphabet_size==3);
 assert(c.total_parent_decompositions==4);
 assert(c.distinct_prefixes==2);
 assert(c.defect_classes==3);
 assert(c.universal_role_states==9);
 assert(c.incidence_polynomial_matches_minpoly_exactly);
 assert(c.incidence_cyclotomic.proved);
 assert(c.incidence_has_no_cyclotomic_factor);
 assert(c.all_pair_fourier_sectors_certified);
 assert(c.no_fourth_generator);
 assert(c.contact_boundary_complete);
 assert(c.simultaneous_three_generator_intertwiner);
 assert(c.boundary_states > 0);
 const auto emit_int_list = [](std::ostream& stream,
                               const std::vector<long long>& values) {
     stream << "[";
     for (std::size_t i = 0; i < values.size(); ++i) {
         if (i != 0) stream << ", ";
         stream << values[i];
     }
     stream << "]";
 };
 const auto emit_bool_list = [](std::ostream& stream,
                                const std::array<bool, 3>& values) {
     stream << "[";
     for (std::size_t i = 0; i < values.size(); ++i) {
         if (i != 0) stream << ", ";
         stream << (values[i] ? "true" : "false");
     }
     stream << "]";
 };
 std::vector<long long> incidence = c.incidence_polynomial;
 std::array<bool, 3> pair_fourier{};
 for (std::size_t i = 0; i < pair_fourier.size(); ++i)
     pair_fourier[i] = c.pair_fourier_audits[i].proved &&
         c.pair_fourier_audits[i].every_twisted_sector_below_untwisted;
 std::ofstream out("lean/generated/supergolden_qrs_audit.lean");
 out << "import Mathlib\n\nnamespace RavelGenerated\n\n"
        "def supergoldenBoundaryStates : Nat := " << c.boundary_states << "\n"
        "def supergoldenBoundaryEdges : Nat := " << c.boundary_edges << "\n"
        "def supergoldenUniversalEdges : Nat := " << c.universal_parent_pair_edges << "\n"
        "def supergoldenRawIncidenceCharacteristic : List Int := ";
 emit_int_list(out, incidence);
 out << "\n"
        "def supergoldenIncidenceCyclotomicFree : Bool := "
     << (c.incidence_has_no_cyclotomic_factor ? "true" : "false") << "\n"
        "def supergoldenPairFourierCertified : List Bool := ";
 emit_bool_list(out, pair_fourier);
 out << "\n\n"
        "theorem supergolden_qrs_audit_valid :\n"
        "    supergoldenBoundaryStates = 20 ∧\n"
        "    supergoldenBoundaryEdges = 25 ∧\n"
        "    supergoldenUniversalEdges = 16 ∧\n"
        "    supergoldenRawIncidenceCharacteristic = [-1, 0, -1, 1] ∧\n"
        "    supergoldenIncidenceCyclotomicFree = true ∧\n"
        "    supergoldenPairFourierCertified = [true, true, true] := by\n"
        "  native_decide\n\n"
        "theorem supergolden_all_pair_twisted_sectors_certified :\n"
        "    ∀ b ∈ supergoldenPairFourierCertified, b = true := by\n"
        "  simp [supergoldenPairFourierCertified]\n\n"
        "end RavelGenerated\n";
 out.close();
 std::cout<<"supergolden Q/R/S closure PASS parents="<<c.total_parent_decompositions
          <<" role_edges="<<c.universal_parent_pair_edges
          <<" classes="<<c.defect_class_edges[0]<<","<<c.defect_class_edges[1]<<","<<c.defect_class_edges[2]<<"\n";
}

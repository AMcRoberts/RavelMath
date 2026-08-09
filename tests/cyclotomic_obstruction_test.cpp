#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "math/proof_reflection.hpp"
#include "ravel/proof/cyclotomic_obstruction.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using mathlib::PolyZ;
using namespace ravel::proof;

static void require(bool x, const char* msg) { if (!x) throw std::runtime_error(msg); }

int main() {
    mathlib::reflection::Trace trace("cyclotomic_obstruction_batch");
    mathlib::reflection::ScopedTrace scope(&trace);

    require(cyclotomic_polynomial(1) == PolyZ{-1,1}, "Phi1");
    require(cyclotomic_polynomial(2) == PolyZ{1,1}, "Phi2");
    require(cyclotomic_polynomial(3) == PolyZ{1,1,1}, "Phi3");
    require(cyclotomic_polynomial(4) == PolyZ{1,0,1}, "Phi4");
    require(cyclotomic_polynomial(6) == PolyZ{1,-1,1}, "Phi6");
    require(cyclotomic_polynomial(12) == PolyZ{1,0,-1,0,1}, "Phi12");

    const PolyZ p = cyclotomic_polynomial(2) * cyclotomic_polynomial(3) * PolyZ{-2,1};
    const auto cert = derive_cyclotomic_obstruction_certificate(p);
    require(cert.proved, "factor certificate");
    require(cert.has_order(2) && cert.has_order(3), "missing factors");
    require(!cert.has_order(1), "spurious Phi1");
    require(cert.remaining_factor == PolyZ{-2,1}, "remaining factor");
    stage_cyclotomic_obstruction(cert, "Phi2*Phi3*(x-2) composite factorization witness");

    const auto sector = derive_z2_character_sector_certificate(PolyZ{-2,1}, PolyZ{1,1});
    require(sector.proved && sector.nontrivial_sector_has_order_two, "Z2 sector");
    const auto matrix_sector = derive_z2_character_sector_certificate(
        std::vector<std::vector<long long>>{{0}},
        std::vector<std::vector<long long>>{{1}});
    require(matrix_sector.proved && matrix_sector.nontrivial_sector_has_order_two,
            "matrix Z2 sector");

    const std::vector<WeightedDirectedEdge> cycle4{{0,1,1},{1,2,1},{2,3,1},{3,0,1}};
    const auto period4 = derive_weighted_cycle_period_certificate(4, cycle4);
    require(period4.proved && period4.period == 4, "period 4");

    const std::vector<WeightedDirectedEdge> weighted{{0,1,2},{1,0,4}};
    const auto period6 = derive_weighted_cycle_period_certificate(2, weighted);
    require(period6.proved && period6.period == 6, "weighted period 6");

    for (std::size_t D=3; D<=64; ++D) {
        std::vector<long long> digits(D,1);
        digits[D-2]=0;
        const auto q = ravel::simple_parry_profile_polynomial(digits);
        const auto c = derive_cyclotomic_obstruction_certificate(q);
        require(c.has_order(2) == (D%2==0), "nearest-left parity detector");
        stage_cyclotomic_obstruction(c, "D="+std::to_string(D)+" nearest-left profile factorization instance");
    }

    auto nodes = trace.find<mathlib::reflection::CyclotomicObstructionReflectionCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " CyclotomicObstructionReflectionCertificate nodes\n";
    assert(nodes.size() == 63);

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("polyOfCoeffs") != std::string::npos);
    assert(lean.find("cyclotomic_obstruction_instance_0") != std::string::npos);

    std::ofstream out("lean/generated/cyclotomic_obstruction.lean");
    out << lean;
    out.close();

    std::cout << "cyclotomic obstruction PASS\n";
}

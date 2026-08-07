// Runs Finding 42's Barge-Diamond pipeline on Finding 37's 10 real
// non-unit Pisot substitutions in Barge's structural class -- newly
// possible after the 2026-08-07 correction to
// barge_diamond_certificate.hpp (the rational-root-theorem check
// proves the ACTUAL root's irrationality at any degree, not just
// degree 2-3; the earlier restriction conflated that with the
// stronger, degree-limited claim "no rational root implies the whole
// polynomial is irreducible", which this certificate never actually
// needed). Substitution data copied verbatim from
// app/probe_barge_class_nonunit_instances.cpp (Finding 37's own
// source), not reconstructed from memory.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/barge_diamond_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("barge_diamond_finding37_batch");
    long long recorded_count = 0;
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        auto run = [&](const char* name, auto cert) {
            std::cout << name << ": irrational=" << cert.beta_irrational << "  " << cert.conclusion << "\n";
            if (cert.beta_irrational) ++recorded_count;
        };

        run("ex1", certify_barge_diamond<4>({
            std::vector<long long>{2,3,1,0}, std::vector<long long>{1,0,3,0},
            std::vector<long long>{3,3,0,0}, std::vector<long long>{0,3,0}}));

        run("ex2", certify_barge_diamond<4>({
            std::vector<long long>{3,2,0,0}, std::vector<long long>{2,3,3,0,0},
            std::vector<long long>{1,3,3,2,0}, std::vector<long long>{0,1,3,0}}));

        run("ex3", certify_barge_diamond<4>({
            std::vector<long long>{3,1,0,0}, std::vector<long long>{1,2,0},
            std::vector<long long>{0,2,0}, std::vector<long long>{2,0,2,0}}));

        run("ex4", certify_barge_diamond<4>({
            std::vector<long long>{3,1,2,0}, std::vector<long long>{2,1,2,0},
            std::vector<long long>{0,2,2,0}, std::vector<long long>{1,2,0}}));

        run("ex5", certify_barge_diamond<4>({
            std::vector<long long>{2,1,0}, std::vector<long long>{3,2,0},
            std::vector<long long>{0,3,0}, std::vector<long long>{1,3,2,0}}));

        run("ex6", certify_barge_diamond<4>({
            std::vector<long long>{3,0,1,0}, std::vector<long long>{1,3,2,0},
            std::vector<long long>{2,3,0}, std::vector<long long>{0,0,0}}));

        run("ex7", certify_barge_diamond<4>({
            std::vector<long long>{0,1,2,0}, std::vector<long long>{2,3,2,0},
            std::vector<long long>{1,0,3,0}, std::vector<long long>{3,1,0}}));

        run("ex8", certify_barge_diamond<4>({
            std::vector<long long>{2,3,1,0}, std::vector<long long>{0,1,3,0},
            std::vector<long long>{1,0,1,0}, std::vector<long long>{3,1,0}}));

        run("ex9", certify_barge_diamond<5>({
            std::vector<long long>{1,1,3,0}, std::vector<long long>{2,4,0},
            std::vector<long long>{0,4,4,0}, std::vector<long long>{3,1,0},
            std::vector<long long>{4,3,0}}));

        run("ex10", certify_barge_diamond<5>({
            std::vector<long long>{2,4,3,0}, std::vector<long long>{4,1,0,0},
            std::vector<long long>{3,4,4,0}, std::vector<long long>{1,2,3,0},
            std::vector<long long>{0,1,0}}));
    }

    auto nodes = trace.find<mathlib::reflection::IntegerEigenvectorNoWitness>();
    std::cout << "\ntrace recorded " << nodes.size() << " IntegerEigenvectorNoWitness nodes "
              << "(" << recorded_count << " certificates reported beta_irrational)\n";
    assert(static_cast<long long>(nodes.size()) == recorded_count);

    std::string lean = render_reflective_lean_module(trace);
    std::ofstream out("/tmp/barge_diamond_finding37_generated.lean");
    out << lean;
    out.close();
    std::cout << "wrote /tmp/barge_diamond_finding37_generated.lean (" << lean.size() << " bytes)\n";
    return 0;
}

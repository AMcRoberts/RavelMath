#include <cassert>
#include <fstream>
#include <iostream>
#include "math/proof_reflection.hpp"
#include "ravel/proof/shift_branch_three_generator_continuation.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
int main(){
  mathlib::reflection::Trace trace("shift_branch_three_generator_continuation_batch");
  {
    mathlib::reflection::ScopedTrace scope(&trace);
    for(std::size_t D=2;D<=256;++D){
      auto c=ravel::proof::derive_shift_branch_three_generator_continuation(D);
      assert(c.proved); assert(c.parent_occurrences==D+1);
      assert(c.ordered_prefix_pair_counts[1]==D && c.ordered_prefix_pair_counts[2]==D);
      assert(c.ordered_prefix_pair_counts[0]+2*D==(D+1)*(D+1));
      ravel::proof::stage_shift_branch_three_generator_continuation(
          c, "D="+std::to_string(D)+" shift-branch three-generator continuation instance");
    }
  }

  auto nodes = trace.find<mathlib::reflection::ShiftBranchThreeGeneratorContinuationReflectionCertificate>();
  std::cout << "trace recorded " << nodes.size()
            << " ShiftBranchThreeGeneratorContinuationReflectionCertificate nodes\n";
  assert(nodes.size() == 255);

  std::string lean = ravel::proof::render_reflective_lean_module(trace);
  assert(lean.find("three_generator_word_induction") != std::string::npos);
  assert(lean.find("shift_branch_three_generator_continuation_instance_0") != std::string::npos);

  std::ofstream out("lean/generated/shift_branch_three_generator_continuation.lean");
  out << lean;
  out.close();

  std::cout<<"shift-branch three-generator continuation PASS\n";
}

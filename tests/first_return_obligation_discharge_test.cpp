#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>
#include "ravel/proof/first_return_obligation_discharge.hpp"
int main(){
 using namespace ravel::proof;
 for(std::size_t n: {3ul,4ul}){
  std::vector<std::filesystem::path> files={"out/cover_tube_defect_splice/n"+std::to_string(n)+"_M2.json"};
  if(n==3) files.emplace_back("out/cover_tube_defect_splice/n3_M3.json");
  auto r=discharge_first_return_obligations(n,files);
  std::cout<<render_first_return_obligation_report(r);
  assert(r.realized_segment_language_complete);
  assert(r.accepted_path_affine_sound);
 }
 std::cout<<"first return obligation discharge PASS\n";
}

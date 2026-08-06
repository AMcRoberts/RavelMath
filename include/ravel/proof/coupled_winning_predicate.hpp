#pragma once
#include <cstddef>
#include <stdexcept>
#include <vector>
namespace ravel::proof {
struct PredicateTreeNode { int feature=-1; long long threshold=0; int left=-1; int right=-1; bool leaf=false; bool value=false; };
inline bool evaluate_predicate_tree(const std::vector<PredicateTreeNode>& nodes, const std::vector<long long>& features) {
  if (nodes.empty()) throw std::invalid_argument("empty predicate tree");
  int i=0;
  for (std::size_t fuel=0; fuel<=nodes.size(); ++fuel) {
    const auto& n=nodes.at(static_cast<std::size_t>(i));
    if (n.leaf) return n.value;
    if (n.feature < 0 || static_cast<std::size_t>(n.feature)>=features.size()) throw std::out_of_range("feature");
    i = features[static_cast<std::size_t>(n.feature)] <= n.threshold ? n.left : n.right;
    if (i < 0 || static_cast<std::size_t>(i)>=nodes.size()) throw std::out_of_range("child");
  }
  throw std::runtime_error("cyclic predicate tree");
}
}

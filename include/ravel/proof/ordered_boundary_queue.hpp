#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include "ravel/proof/block_height_shell_rank.hpp"
namespace ravel::proof {
struct OrderedBoundaryQueue {
  std::vector<std::int64_t> left;   // distance 0,1,... from the outgoing boundary
  std::vector<std::int64_t> right;  // distance 0,1,... from the incoming boundary
};
inline OrderedBoundaryQueue ordered_boundary_queue(const ShellState& x, std::size_t depth) {
  if (x.empty() || depth > x.size()) throw std::invalid_argument("invalid boundary depth");
  OrderedBoundaryQueue q; q.left.reserve(depth); q.right.reserve(depth);
  for (std::size_t k=0;k<depth;++k) { q.left.push_back(x[k]); q.right.push_back(x[x.size()-1-k]); }
  return q;
}
inline ShellState shift_append(const ShellState& x, std::int64_t incoming) {
  if (x.empty()) throw std::invalid_argument("empty state");
  ShellState y(x.begin()+1,x.end()); y.push_back(incoming); return y;
}
struct BoundaryQueueUpdate {
  OrderedBoundaryQueue before, after;
  std::int64_t incoming=0;
  std::int64_t left_entering=0;
  bool exact=false;
};
inline BoundaryQueueUpdate derive_boundary_queue_update(const ShellState& x, std::int64_t incoming, std::size_t depth) {
  auto y=shift_append(x,incoming); auto b=ordered_boundary_queue(x,depth); auto a=ordered_boundary_queue(y,depth);
  bool ok=true;
  // Left queue shifts toward the outgoing boundary: L'_k=L_{k+1}, where available.
  for(std::size_t k=0;k+1<depth;++k) ok &= a.left[k]==b.left[k+1];
  // Right queue receives one new item and pushes old items inward: R'_0=incoming, R'_k=R_{k-1}.
  if(depth){ok &= a.right[0]==incoming; for(std::size_t k=1;k<depth;++k) ok &= a.right[k]==b.right[k-1];}
  const auto left_entering = depth < x.size() ? x[depth] : incoming;
  if (depth) ok &= a.left[depth-1] == left_entering;
  return {std::move(b),std::move(a),incoming,left_entering,ok};
}
// Weighted boundary potential. Its one-step difference is the exact telescoping law.
inline std::int64_t weighted_boundary_value(const OrderedBoundaryQueue& q,
                                             const std::vector<std::int64_t>& left_w,
                                             const std::vector<std::int64_t>& right_w) {
  if(q.left.size()!=left_w.size()||q.right.size()!=right_w.size()) throw std::invalid_argument("weight size");
  std::int64_t z=0; for(std::size_t k=0;k<left_w.size();++k) z+=left_w[k]*q.left[k]+right_w[k]*q.right[k]; return z;
}
inline std::int64_t weighted_boundary_delta_formula(const OrderedBoundaryQueue& q,
                                                     std::int64_t incoming,
                                                     std::int64_t left_entering,
                                                     const std::vector<std::int64_t>& left_w,
                                                     const std::vector<std::int64_t>& right_w) {
  const auto d=q.left.size(); if(d!=left_w.size()||d!=right_w.size()) throw std::invalid_argument("weight size");
  std::int64_t z=0;
  // left: shift toward the outgoing boundary and insert one new item at depth d-1.
  for(std::size_t k=0;k+1<d;++k) z += left_w[k]*q.left[k+1];
  if(d) z += left_w[d-1]*left_entering;
  for(std::size_t k=0;k<d;++k) z -= left_w[k]*q.left[k];
  // right: w_0*incoming + sum_{k=1}^{d-1} w_k R_{k-1} - sum_k w_k R_k.
  if(d) z += right_w[0]*incoming;
  for(std::size_t k=1;k<d;++k) z += right_w[k]*q.right[k-1];
  for(std::size_t k=0;k<d;++k) z -= right_w[k]*q.right[k];
  return z;
}
}

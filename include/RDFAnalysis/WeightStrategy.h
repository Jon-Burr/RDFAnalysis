#ifndef RDFAnalysis_WeightStrategy_H
#define RDFAnalysis_WeightStrategy_H

#include <type_traits>

/**
 * @file WeightStrategy.h
 * @brief File containing the \ref WeightStrategy enum class
 */

namespace RDFAnalysis {
  enum class WeightStrategy {
    /**
     * @brief enum class to describe how weights are applied.
     *
     * There are two dimensions to the weighting strategy: whether the weight
     * should be applied in 'data' modes or only in the 'MC' mode and whether or
     * not the actual weight should be the product of the given one and any
     * pre-existing weight.
     *
     * Most weights are scale factors, which obey the strategy
     * WeightStrategy::Multiplicative | WeightStrategy::MCOnly as scale factors
     * are multiplicative (where they are unrelated, each individual scale
     * factor should be applied only once on any event) and only applied to MC.
     *
     * Weights applied in 'data' mode are typically specialised histogram
     * weights (e.g. pt-weighted eta-phi plots).
     *
     * If a weight should not be multiplicative and should be applied in both
     * modes you would normally need to call this
     * ~(WeightStrategy::Multiplicative | WeightStrategy::MC), however there is
     * also a WeightStrategy::Null option which describes this.
     */
    Null = 0, /**< Helper, meaning neither multiplicative nor MC-only */
    Multiplicative = 1 << 0, /**< Multiply by the existing weight */
    MCOnly = 1 << 1, /**< The weight should only be applied in the MC-mode */
    Default = Multiplicative | MCOnly /**< Default */
  }; //> end enum class WeightStrategy

  /// Bitwise OR of two strategies
  WeightStrategy operator|(WeightStrategy lhs, WeightStrategy rhs);

  /// Switch on the bits of lhs that are in rhs
  WeightStrategy& operator|=(WeightStrategy& lhs, WeightStrategy rhs);

  /// Bitwise AND of two strategies
  WeightStrategy operator&(WeightStrategy lhs, WeightStrategy rhs);

  /// Switch off any bits of lhs not in rhs
  WeightStrategy& operator&=(WeightStrategy& lhs, WeightStrategy rhs);

  /// Bitwise XOR of two strategies
  WeightStrategy operator^(WeightStrategy lhs, WeightStrategy rhs);

  /// Flip any bits of rhs that are in rhs
  WeightStrategy& operator^=(WeightStrategy& lhs, WeightStrategy rhs);

  /// Negate a value
  WeightStrategy operator~(WeightStrategy ws);

  /// Contextually convert to a bool
  bool operator !(WeightStrategy ws);

} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_WeightStrategy_H

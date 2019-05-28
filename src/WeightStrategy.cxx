#include "RDFAnalysis/WeightStrategy.h"

namespace RDFAnalysis {
  /// Bitwise OR of two strategies
  WeightStrategy operator|(WeightStrategy lhs, WeightStrategy rhs)
  {
    return static_cast<WeightStrategy>(
        static_cast<std::underlying_type_t<WeightStrategy>>(lhs) |
        static_cast<std::underlying_type_t<WeightStrategy>>(rhs) );
  }

  /// Switch on the bits of lhs that are in rhs
  WeightStrategy& operator|=(WeightStrategy& lhs, WeightStrategy rhs)
  {
    return lhs = lhs | rhs;
  }

  /// Bitwise AND of two strategies
  WeightStrategy operator&(WeightStrategy lhs, WeightStrategy rhs)
  {
    return static_cast<WeightStrategy>(
        static_cast<std::underlying_type_t<WeightStrategy>>(lhs) &
        static_cast<std::underlying_type_t<WeightStrategy>>(rhs) );
  }

  /// Switch off any bits of lhs not in rhs
  WeightStrategy& operator&=(WeightStrategy& lhs, WeightStrategy rhs)
  {
    return lhs = lhs & rhs;
  }

  /// Bitwise XOR of two strategies
  WeightStrategy operator^(WeightStrategy lhs, WeightStrategy rhs)
  {
    return static_cast<WeightStrategy>(
        static_cast<std::underlying_type_t<WeightStrategy>>(lhs) ^
        static_cast<std::underlying_type_t<WeightStrategy>>(rhs) );
  }

  /// Flip any bits of rhs that are in rhs
  WeightStrategy& operator^=(WeightStrategy& lhs, WeightStrategy rhs)
  {
    return lhs = lhs ^ rhs;
  }

  /// Negate a value
  WeightStrategy operator~(WeightStrategy ws)
  {
    return static_cast<WeightStrategy>(
        ~static_cast<std::underlying_type_t<WeightStrategy>>(ws) );
  }

  /// Contextually convert to a bool
  bool operator !(WeightStrategy ws)
  {
    return ws == WeightStrategy::Null;
  }
} //> end namespace RDFAnalysis

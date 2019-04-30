#ifndef RDFAnalysis_NodeStatistics_H
#define RDFAnalysis_NodeStatistics_H

#include "ROOT/RDF/ActionHelpers.hxx"

namespace RDFAnalysis {
  /**
   * @brief Class recording the number of events which this node has seen
   */
  class NodeStatistics :
    public ROOT::Detail::RDF::RActionImpl<NodeStatistics> {
    public:
      /// Declare the column types to RAction
      using ColumnTypes_t = ROOT::TypeTraits::TypeList<>;
      /// Declare the output type to RAction
      using Result_t = ULong64_t;
      /// Constructor
      NodeStatistics(
          const std::shared_ptr<ULong64_t>& result,
          unsigned int nSlots);
      NodeStatistics(NodeStatistics&&) = default;

      void InitTask(TTreeReader*, unsigned int) {}
      /// Process one event
      void Exec(unsigned int slot);
      void Initialize() {}
      /// Combined the results of each thread
      void Finalize();
      /// Return the current count
      ULong64_t& PartialUpdate(unsigned int slot) { return m_resultSlots[slot]; }
      /// Name the action
      std::string GetActionName() { return "NodeStatistics"; }
      /// Get the result
      std::shared_ptr<ULong64_t> GetResultPtr() { return m_result; }
    private:
      std::shared_ptr<ULong64_t> m_result;
      ROOT::Internal::RDF::Results<ULong64_t> m_resultSlots;
    };
  /**
   * @brief Class recording the weighted number of events which this node has
   * seen.
   * The result type is pair<float, float> where the first entry is the sum of
   * weights and the second the sum of weights squared.
   */
  class WeightedNodeStatistics :
    public ROOT::Detail::RDF::RActionImpl<NodeStatistics> {
    public:
      /// Declare the column types to RAction
      using ColumnTypes_t = ROOT::TypeTraits::TypeList<>;
      /// Declare the output type to RAction
      using Result_t = std::pair<float, float>;
      /// Constructor
      WeightedNodeStatistics(
          const std::shared_ptr<Result_t>& result,
          unsigned int nSlots);
      WeightedNodeStatistics(WeightedNodeStatistics&&) = default;

      void InitTask(TTreeReader*, unsigned int) {}
      /// Process one event
      void Exec(unsigned int slot, float weight);
      void Initialize() {}
      /// Combined the results of each thread
      void Finalize();
      /// Return the current count
      Result_t& PartialUpdate(unsigned int slot) { return m_resultSlots[slot]; }
      /// Name the action
      std::string GetActionName() { return "WeightedNodeStatistics"; }
      /// Get the result
      std::shared_ptr<Result_t> GetResultPtr() { return m_result; }
    private:
      std::shared_ptr<Result_t> m_result;
      ROOT::Internal::RDF::Results<Result_t> m_resultSlots;
    };
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_NodeStatistics_H

#ifndef RDFAnalysis_NodeBase_H
#define RDFAnalysis_NodeBase_H

// Package includes
#include "RDFAnalysis/IBranchNamer.h"
#include "RDFAnalysis/Helpers.h"
#include "RDFAnalysis/SysResultPtr.h"
#include "RDFAnalysis/SysVar.h"

// ROOT includes
#include "ROOT/RDataFrame.hxx"
#include <TObject.h>

// STL includes
#include <string>
#include <map>
#include <memory>

namespace RDFAnalysis {
  using RNode = ROOT::RDF::RNode;
  using ColumnNames_t = ROOT::RDataFrame::ColumnNames_t;

  class NodeBase
  {
    public:
      /**
       * @brief Define a new variable on this node
       * @tparam F The functor type
       * @param name The name of the column to define
       * @param f The functor
       * @param columns The input variables (if any) to the functor
       * @return A non-owning pointer to this object.
       * The new column's data type will be the return type of the functor
       */
      template <typename F>
          enable_ifn_string_t<F, NodeBase*> Define(
            const std::string& name,
            F f,
            const ColumnNames_t& columns = {});

      /**
       * @brief Define a new variable on this node
       * @param name The name of the column to define
       * @param expression The string expression to interpret
       * @return A non-owning pointer to this object.
       * The new column's data type will be the return type of the JITted
       * function
       */
      NodeBase* Define(
          const std::string& name,
          const std::string& expression);

      /**
       * @brief Define a new variable on this node
       * @param name The name of the column to define
       * @param expression The string expression to interpret
       * @param columns The input variables to the expression
       * @return A non-owning pointer to this object.
       * The new column's data type will be the return type of the JITted
       * function. The expression should have the column names replaced by
       * placeholders like {idx} (where idx is the index of the branch in the
       * columns vector).
       */
      NodeBase* Define(
          const std::string& name,
          const std::string& expression,
          const ColumnNames_t& columns);
      /**
       * @brief Get the name of the weight branch.
       * The name returned will be the base name, not resolved for any
       * systematic variation. If there is no weight set the empty string will
       * be returned.
       */
      const std::string& getWeight() const { return m_weight; }

      /**
       * @brief Fill an object on each event
       * @tparam T The type of object to be filled.
       * @param model The 'model' object to fill.
       * @param columns The columns to use for the object's Fill method.
       *
       * Note that right now this won't work if T doesn't inherit from TH1. TODO
       * fix this! 
       */
      template <typename T>
        SysResultPtr<T> Fill(
            const T& model,
            const ColumnNames_t& columns);

      template <typename T, typename... TrArgs, typename... Args>
        std::map<std::string, T> Act(
            std::function<T(RNode&, TrArgs...)> f,
            const ColumnNames_t& columns,
            Args&&... args);

      template <typename F, typename... Args>
        std::map<std::string, typename ROOT::TTraits::CallableTraits<F>::ret_type> Act(
            F&& f,
            const ColumnNames_t& columns,
            Args&&... args)
        {
          return Act(
              std::function<typename ROOT::TTraits::CallableTraits<F>::ret_type(RNode&, typename sysvar_traits<Args&&>::value_type...)>(f),
              columns,
              std::forward<Args>(args)...);
        }

      template <typename T, typename... TrArgs, typename... Args>
        std::map<std::string, T> Act(
            T (RNode::*f)(TrArgs&&...),
            const ColumnNames_t& columns,
            Args&&... args);

      /// Get the name
      const std::string& name() const { return m_name; }

      /// Get the name in a cutflow
      const std::string& cutflowName() const { return m_cutflowName; }

      /// Is this anonymous?
      bool isAnonymous() const { return m_name.empty(); }

      /// Get the RNode objects
      const std::map<std::string, RNode>& rnodes() const { return m_rnodes; }
      std::map<std::string, RNode>& rnodes() { return m_rnodes; }

      /// The namer
      const IBranchNamer& namer() const { return *m_namer; }

      /// Get the ROOT RNode
      const RNode& rootRNode() const { return *m_rootRNode; }

      /// Get the ROOT RNode TODO only temporary
      RNode& rootRNode() { return *m_rootRNode; }

      /* Node(Node&&) = default; */

      /// Iterate over the objects defined on this
      auto objects() { return as_range(m_objects); }
      /// (Const) iterate over all the objects defined on this
      auto objects() const { return as_range(m_objects); }

      /// Get the node statistics
      SysResultPtr<ULong64_t> stats() { return m_stats; }
      /// Get the weighted statistics
      SysResultPtr<std::pair<float, float>> weightedStats()
      { return m_weightedStats; }

      /// Is the node the root?
      virtual bool isRoot() const = 0;

    protected:
      struct NamerInitialiser {
        NamerInitialiser() {} // no-op
        NamerInitialiser(
            IBranchNamer& namer,
            const std::map<std::string, ROOT::RDF::RNode>& rnodes) {
          namer.readBranchList(rnodes);
        }
      };

      /**
       * @brief Create child RNodes to be used for a filter from this node
       * @tparam F The functor type
       * @param f The functor
       * @param columns The input variables to the functor
       */
      template <typename F>
        enable_ifn_string_t<F, std::map<std::string, RNode>> makeChildRNodes(
            F f,
            const ColumnNames_t& columns = {},
            const std::string& cutflowName = "");

      /**
       * @brief Create child RNodes to be used for a filter from this node
       * @param expression The expression to describe the filter
       */
      std::map<std::string, RNode> makeChildRNodes(
          const std::string& expression,
          const std::string& cutflowName = "");

      /**
       * @brief Create child RNodes to be used for a filter from this node
       * @param expression The expression to describe the filter
       * @param columns The input variables to the expression
       * The expression should have the column names replaced by placeholders
       * like {idx} (where idx is the index of the branch in the columns
       * vector).
       */
      std::map<std::string, RNode> makeChildRNodes(
          const std::string& expression,
          const ColumnNames_t& columns,
          const std::string& cutflowName = "");

      /**
       * @brief Create the root node of the tree
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param weight Expression to calculate a weight.
       */
      NodeBase(
          const RNode& rnode,
          std::unique_ptr<IBranchNamer>&& namer,
          const std::string& name = "ROOT",
          const std::string& cutflowName = "Number of events",
          const std::string& weight = "");

      /**
       * @brief Create the root node of the tree
       * @tparam W The functor used to calculate the weight
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param w Functor used to calculate the weight
       * @param columns THe input columns for the weight
       */
      template <typename W>
        NodeBase(
            const RNode& rnode,
            std::unique_ptr<IBranchNamer>&& namer,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& columns);

      /**
       * @brief Create a child node
       * @param parent The parent of this node
       * @param rnodes This node's RNodes
       * @param name The name of this node
       * @param cutflowName How this node appears in cutflows
       */
      NodeBase(
          NodeBase& parent,
          std::map<std::string, RNode>&& rnodes,
          const std::string& name,
          const std::string& cutflowName);

      /**
       * @brief Set the weight on this node
       * @tparam F the functor type
       * @param f The functor
       * @param columns The input columns to \ref f (if any)
       * @param parentWeight The name of the parent weight branch. If this
       * weight should not be multiplied by the parent this will be the empty
       * string.
       * @return The name of the new weight
       * The new weight will be calculated and stored in a new branch.
       */
      template <typename F>
        enable_ifn_string_t<F, std::string> setWeight(
            F f,
            const ColumnNames_t& columns = {},
            const std::string& parentWeight = "");

      /**
       * @brief Set the weight on this node
       * @param expression The expression to calculate the weight
       * @param parentWeight The name of the parent weight branch. If this
       * weight should not be multiplied by the parent this will be the empty
       * string.
       * @return The name of the new weight
       * The new weight will be calculated and stored in a new branch.
       */
      std::string setWeight(
          const std::string& expression,
          const std::string& parentWeight);

      /// Internal function to name the weight branch
      std::string nameWeight();

      /// Internal function to setup the weighted statistics
      void setupWeightedStatistics();

      /// The RNode objects, keyed by systematic
      std::map<std::string, RNode> m_rnodes;      

      /// The branch namer
      std::unique_ptr<IBranchNamer> m_namer;

      /// Helper struct to force early initialisation of the namer
      NamerInitialiser m_namerInit;

      /// The Node's name
      std::string m_name;

      /// The name in the cutflow
      std::string m_cutflowName;

      /// Keep a pointer to the ROOT RNode of the whole chain
      RNode* m_rootRNode = nullptr;

      /// The weight on this node
      std::string m_weight;

      /// Any TObject pointers declared on this
      std::vector<SysResultPtr<TObject>> m_objects;

      /// The node statistics
      SysResultPtr<ULong64_t> m_stats;

      /// The node statistics (including weights)
      SysResultPtr<std::pair<float, float>> m_weightedStats;
  }; //> end class NodeBase
} //> end namespace RDFAnalysis
#include "RDFAnalysis/NodeBase.icc"
#endif //> !RDFAnalysis_NodeBase_H

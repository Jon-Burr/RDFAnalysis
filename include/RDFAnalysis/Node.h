#ifndef RDFAnalysis_Node_H
#define RDFAnalysis_Node_H

// Package includes
#include "RDFAnalysis/IBranchNamer.h"
#include "RDFAnalysis/Helpers.h"
#include "RDFAnalysis/SysResultPtr.h"
#include "RDFAnalysis/NodeStatistics.h"

// ROOT includes
#include "ROOT/RDataFrame.hxx"
#include <TObject.h>

// STL includes
#include <string>
#include <map>
#include <memory>

namespace RDFAnalysis {
  using RNode = ROOT::RDF::RNode;
  class Node : public std::enable_shared_from_this<Node> 
  {
    public:
      /// Typedefs
      using ColumnNames_t = ROOT::RDataFrame::ColumnNames_t;

      /**
       * @brief Define a new variable on this node
       * @tparam F The functor type
       * @param name The name of the column to define
       * @param f The functor
       * @param columns The input variables (if any) to the functor
       * @return A reference to this object.
       * The new column's data type will be the return type of the functor
       */
      template <typename F>
        std::enable_if_t<!std::is_convertible<F, std::string>{},Node&> Define(
            const std::string& name,
            F f,
            const ColumnNames_t& columns = {});

      /**
       * @brief Define a new variable on this node
       * @param name The name of the column to define
       * @param expression The string expression to interpret
       * @return A reference to this object.
       * The new column's data type will be the return type of the JITted
       * function
       */
      Node& Define(
          const std::string& name,
          const std::string& expression);

      /**
       * @brief Define a new variable on this node
       * @param name The name of the column to define
       * @param expression The string expression to interpret
       * @param columns The input variables to the expression
       * @return A reference to this object.
       * The new column's data type will be the return type of the JITted
       * function. The expression should have the column names replaced by
       * placeholders like {idx} (where idx is the index of the branch in the
       * columns vector).
       */
      Node& Define(
          const std::string& name,
          const std::string& expression,
          const ColumnNames_t& columns);

      /**
       * @brief Create a filter on this node
       * @tparam F The functor type
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       */
      template <typename F>
        std::enable_if_t<!std::is_convertible<F, std::string>{},
        std::shared_ptr<Node>> Filter(
            F f,
            const ColumnNames_t& columns = {},
            const std::string& name = "",
            const std::string& cutflowName = "");

      /**
       * @brief Create a filter on this node
       * @param expression The expression to describe the filter
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       */
      std::shared_ptr<Node> Filter(
          const std::string& expression,
          const std::string& name = "",
          const std::string& cutflowName = "");

      /**
       * @brief Create a filter on this node
       * @param expression The expression to describe the filter
       * @param columns The input variables to the expression
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       * The expression should have the column names replaced by placeholders
       * like {idx} (where idx is the index of the branch in the columns
       * vector).
       */
      std::shared_ptr<Node> Filter(
          const std::string& expression,
          const ColumnNames_t& columns,
          const std::string& name = "",
          const std::string& cutflowName = "");

      /**
       * @brief Set the weight on this node
       * @tparam F the functor type
       * @param f The functor
       * @param columns The input columns to \ref f (if any)
       * @param multiplicative Whether to multiply by the weight (if any) set on
       * the previous node.
       * @return shared pointer to this
       * The new weight will be calculated and stored in a new branch.
       */
      template <typename F>
        std::enable_if_t<!std::is_convertible<F, std::string>{},
        std::shared_ptr<Node>> setWeight(
            F f,
            const ColumnNames_t& columns = {},
            bool multiplicative = true);

      /**
       * @brief Set the weight on this node
       * @param expression The expression to calculate the weight
       * @param multiplicative Whether to multiply by the weight (if any) set on
       * the previous node.
       * @return shared pointer to this
       * The new weight will be calculated and stored in a new branch.
       */
      std::shared_ptr<Node> setWeight(
          const std::string& expression,
          bool multiplicative = true);

      /**
       * @brief Set the weight on this node
       * @param expression The expression to calculate the weight
       * @param columns The input variables to the expression
       * @param multiplicative Whether to multiply by the weight (if any) set on
       * the previous node.
       * @return shared pointer to this
       * The new weight will be calculated and stored in a new branch.
       * The expression should have the column names replaced by placeholders
       * like {idx} (where idx is the index of the branch in the columns
       * vector).
       */
      std::shared_ptr<Node> setWeight(
          const std::string& expression,
          const ColumnNames_t& columns,
          bool multiplicative = true);

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

      /// Get the name
      const std::string& name() const { return m_name; }

      /// Get the name in a cutflow
      const std::string& cutflowName() const { return m_cutflowName; }

      /// Is this anonymous?
      bool isAnonymous() const { return m_name.empty(); }

      /// Get the RNode objects
      const std::map<std::string, RNode>& rnodes() const { return m_rnodes; }

      /// The namer
      const IBranchNamer& namer() const { return *m_namer; }

      /// Get the ROOT RNode
      const RNode& rootRNode() const { return *m_rootRNode; }

      /// Get the ROOT RNode TODO only temporary
      RNode& rootRNode() { return *m_rootRNode; }

      Node(Node&&) = default;


      /// Allow access to iterate over the child nodes
      auto children() { return as_range(m_children); }
      /// Allow (const) access to iterate over the child nodes
      auto children() const { return as_range(m_children); }

      /// Iterate over the objects defined on this
      auto objects() { return as_range(m_objects); }
      /// (Const) iterate over all the objects defined on this
      auto objects() const { return as_range(m_objects); }

      /// Get the node statistics
      SysResultPtr<NodeStatistics::Result_t> stats() { return m_stats; }
      /// Get the weighted statistics
      SysResultPtr<WeightedNodeStatistics::Result_t> weightedStats()
      { return m_weightedStats; }

      /// Get the parent of this node
      Node* getParent() { return m_parent; }
      const Node* getParent() const { return m_parent; }

      /// Is the node the root?
      bool isRoot() const { return m_parent == nullptr; }

    private:
      friend std::shared_ptr<Node> createROOT(
          const RNode&,
          std::unique_ptr<IBranchNamer>,
          const std::string&,
          const std::string&);

      /**
       * @brief Create the root node of the tree
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param 
       */
      Node(
          const RNode& rnode,
          std::unique_ptr<IBranchNamer> namer,
          const std::string& name = "ROOT",
          const std::string& cutflowName = "Number of events");

      /**
       * @brief Create a child node
       * @param parent The parent of this node
       * @param name The name of this node
       * @param cutflowName How this node appears in cutflows
       */
      Node(
          Node& parent,
          const std::string& name,
          const std::string& cutflowName);

      /// Internal function to name the weight branch
      std::string nameWeight();

      /// Internal function to setup the weighted statistics
      void setupWeightedStatistics();

      /// The parent of this node
      Node* m_parent = nullptr;

      /// The RNode objects, keyed by systematic
      std::map<std::string, RNode> m_rnodes;      

      /// The branch namer
      std::unique_ptr<IBranchNamer> m_namer;

      /// The Node's name
      std::string m_name;

      /// The name in the cutflow
      std::string m_cutflowName;

      /// Keep a pointer to the ROOT RNode of the whole chain
      RNode* m_rootRNode = nullptr;

      /// Any children of this node
      std::vector<std::shared_ptr<Node>> m_children;

      /// Any TObject pointers declared on this
      std::vector<SysResultPtr<TObject>> m_objects;

      /// The node statistics
      SysResultPtr<NodeStatistics::Result_t> m_stats;

      /// The node statistics (including weights)
      SysResultPtr<WeightedNodeStatistics::Result_t> m_weightedStats;

      /// The weight on this node
      std::string m_weight;
  }; //> end class Node

  /**
   * @brief Create the root node of the tree
   * @param rnode The RDataFrame that forms the base of the tree
   * @param namer The branch namer
   * @param name The name of the root node
   * @param cutflowName How the root node appears in the cutflow
   */
  std::shared_ptr<Node> createROOT(
      const RNode& rnode,
      std::unique_ptr<IBranchNamer>  namer,
      const std::string& name = "ROOT",
      const std::string& cutflowName = "Number of events");

} //> end namespace RDFAnalysis
#include "RDFAnalysis/Node.icc"
#endif //> !RDFAnalysis_Node_H

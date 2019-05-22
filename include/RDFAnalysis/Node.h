#ifndef RDFAnalysis_Node_H
#define RDFAnalysis_Node_H

// Package includes
#include "RDFAnalysis/EmptyDetail.h"
#include "RDFAnalysis/NodeBase.h"
#include "RDFAnalysis/NodeFwd.h"

namespace RDFAnalysis {

  /**
   * @brief Class to represent a single step in the analysis process.
   *
   * @tparam Detail Class to contain any extra information attached to the node.
   *
   * For the purposes of this package an analysis is modeled as a tree
   * structure, each selection forming a new node in the tree. Multiple
   * selections can be defined from a single node, forming a branch in the tree
   * at that point. Each node can have attached TObjects (created using the \ref
   * Fill function and accessed using the \ref NodeBase::objects function) as
   * well as additional objects that can be accessed using the \ref details
   * function.
   *
   * The tree structure can be navigated through using the \ref parent and \ref
   * children functions.
   */
  template <typename Detail>
  class Node : public NodeBase
  {
    public:
      // Typedefs
      /// The type of the Detail on this node.
      using detail_t = Detail;

      /**
       * @brief Define a new variable on this node
       * @tparam Args the argument types for the base class function
       * @tparam args the arguments for the base class function
       * @return A non-owning pointer to this object
       * \see NodeBase::Define
       */
      template <typename... Args>
        Node* Define(Args&&... args);

      /**
       * @brief Create a filter on this node
       * @tparam F The functor type
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       */
      template <typename F>
        enable_ifn_string_t<F, Node*> Filter(
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
      Node* Filter(
          const std::string& expression,
          const std::string& name = "",
          const std::string& cutflowName = "");

      /// Allow access to iterate over the child nodes
      auto children() { return as_range(m_children); }
      /// Allow (const) access to iterate over the child nodes
      auto children() const { return as_range(m_children); }

      /// Get the node details
      Detail& detail() { return m_detail; }
      /// (Const) get the node details
      const Detail& detail() const { return m_detail; }

      /// Get the parent of this node
      Node* parent() { return m_parent; }
      const Node* parent() const { return m_parent; }

      /// Is the node the root?
      bool isRoot() const { return m_parent == nullptr; }

      /**
       * @brief Create the root node of the tree
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param weight Expression to calculate a weight.
       */
      static std::unique_ptr<Node> createROOT(
          const RNode& rnode,
          std::unique_ptr<IBranchNamer>&& namer,
          const std::string& name = "ROOT",
          const std::string& cutflowName = "Number of events",
          const std::string& weight = "")
      {
        return std::unique_ptr<Node>(
            new Node(rnode, std::move(namer), name, cutflowName, weight) );
      }

    private:
      /**
       * @brief Create the root node of the tree
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param weight Expression to calculate a weight.
       */
      Node(
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
        Node(
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
      Node(
          Node& parent,
          std::map<std::string, RNode>&& rnodes,
          const std::string& name,
          const std::string& cutflowName);

      /// The parent of this node
      Node* m_parent = nullptr;

      /// Any children of this node
      std::vector<std::unique_ptr<Node>> m_children;

      /// The node's details
      Detail m_detail;
  }; //> end class Node

} //> end namespace RDFAnalysis
#include "RDFAnalysis/Node.icc"
#endif //> !RDFAnalysis_Node_H

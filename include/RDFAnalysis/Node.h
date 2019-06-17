#ifndef RDFAnalysis_Node_H
#define RDFAnalysis_Node_H

// Package includes
#include "RDFAnalysis/EmptyDetail.h"
#include "RDFAnalysis/NodeBase.h"
#include "RDFAnalysis/NodeFwd.h"

/**
 * @file Node.h
 * @brief File containing the central analysis class.
 */

namespace RDFAnalysis {

  /**
   * @brief Class to represent a single step in the analysis process.
   *
   * @tparam Detail Class to contain any extra information attached to the node.
   *
   * For the purposes of this package an analysis is modeled as a tree
   * structure, each selection forming a new node in the tree. Multiple
   * selections can be defined from a single node, forming a branch in the tree
   * at that point. Each node can have attached TObjects (created using the Fill
   * function and accessed using the NodeBase::objects function) as well as
   * additional objects that can be accessed using the details function.
   *
   * The tree structure can be navigated through using the parent and children
   * functions.
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
       * @tparam F The functor type
       * @param name The name of the column to define
       * @param f The functor
       * @param columns The input variables (if any) to the functor
       * @return A non-owning pointer to this object.
       *
       * The new column's data type will be the return type of the functor
       */
      template <typename F>
          enable_ifn_string_t<F, Node*> Define(
            const std::string& name,
            F f,
            const ColumnNames_t& columns = {})
          { 
            NodeBase::Define(name, f, columns);
            return this;
          }

      /**
       * @brief Define a new variable on this node
       * @param name The name of the column to define
       * @param expression The string expression to interpret
       * @return A non-owning pointer to this object.
       *
       * The new column's data type will be the return type of the JITted
       * function
       */
      Node* Define(
          const std::string& name,
          const std::string& expression)
      {
        NodeBase::Define(name, expression);
        return this;
      }

      /**
       * @brief Define a new variable on this node
       * @param name The name of the column to define
       * @param expression The string expression to interpret
       * @param columns The input variables to the expression
       * @return A non-owning pointer to this object.
       *
       * The new column's data type will be the return type of the JITted
       * function. The expression should have the column names replaced by
       * placeholders like {idx} (where idx is the index of the branch in the
       * columns vector).
       */
      Node* Define(
          const std::string& name,
          const std::string& expression,
          const ColumnNames_t& columns)
      {
        NodeBase::Define(name, expression, columns);
        return this;
      }

      /**
       * @brief Define several new variables on this node in a single statement.
       * @tparam F The functor type
       * @tparam Ret_t The return type of the functor - must be a tuple
       * @tparam N The number of defined arguments
       * @param names The names of the defined variables
       * @param f The functor
       * @param columns  The inputs to the functor
       * @return a non-owning pointer to this object.
       *
       * Use this function where you need to return multiple variables from a
       * single function. Note that the affecting systematics will be the same
       * for all of the returned objects so make sure that this is appropriate.
       * The functor should return a std::tuple with N different types and the
       * names parameter should have exactly the same number of entries (else
       * the code will not compile).
       */
      template <
        std::size_t N,
        typename F,
        typename Ret_t = typename ROOT::TTraits::CallableTraits<F>::ret_type>
        std::enable_if_t<N==std::tuple_size<Ret_t>::value, Node>* Define(
            const std::array<std::string, N>& names,
            F f,
            const ColumnNames_t& columns)
        {
          NodeBase::Define(names, f, columns);
          return this;
        }

      /**
       * @brief Create a filter on this node
       * @tparam F The functor type
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       * @param strategy Weighting strategy for this weight
       *
       * In this overload the functor calculates the pass decision and the
       * weight in one go, return std::make_tuple(pass, weight).
       */
      template <typename F>
        std::enable_if_t<std::is_convertible<typename ROOT::TTraits::CallableTraits<F>::ret_type, std::tuple<bool, float>>::value, Node*> Filter(
            F f,
            const ColumnNames_t& columns = {},
            const std::string& name = "",
            const std::string& cutflowName = "",
            WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Create a filter on this node
       * @tparam F The functor type
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       * @param weight Expression to calculate the node weight
       * @param strategy Weighting strategy for this weight
       */
      template <typename F>
        std::enable_if_t<std::is_convertible<typename ROOT::TTraits::CallableTraits<F>::ret_type, bool>::value, Node*> Filter(
            F f,
            const ColumnNames_t& columns = {},
            const std::string& name = "",
            const std::string& cutflowName = "",
            const std::string& weight = "",
            WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Create a filter on this node
       * @param expression The expression to describe the filter
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       * @param weight Expression to calculate the node weight
       * @param strategy Weighting strategy for this weight
       */
      Node* Filter(
          const std::string& expression,
          const std::string& name = "",
          const std::string& cutflowName = "",
          const std::string& weight = "",
          WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Create a filter on this node
       * @tparam F The functor type
       * @tparam W The functor type used for the weight
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       * @param w The functor used to calculate the weight
       * @param weightColumns The input variables to the weight functor
       * @param strategy Weighting strategy for this weight
       */
      template <typename F, typename W>
        std::enable_if_t<!std::is_convertible<F, std::string>::value && !std::is_convertible<W, std::string>::value, Node*> Filter(
            F f,
            const ColumnNames_t& columns,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& weightColumns = {},
            WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Create a filter on this node
       * @tparam W The functor type used for the weight
       * @param expression The expression to describe the filter
       * @param name The name of the new node
       * @param cutflowName How the new node appears in the cutflow
       * @param w The functor used to calculate the weight
       * @param weightColumns The input variables to the weight functor
       * @param strategy Weighting strategy for this weight
       */
      template <typename W>
        enable_ifn_string_t<W, Node*> Filter(
            const std::string& expression,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& weightColumns = {},
            WeightStrategy strategy = WeightStrategy::Default);

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
      /// (const) get the parent of this node
      const Node* parent() const { return m_parent; }

      /// Is the node the root?
      bool isRoot() const { return m_parent == nullptr; }

      /**
       * @brief Create the root node of the tree
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param isMC Whether or not MC mode (weighting) should be used
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param weight Expression to calculate a weight.
       * @param strategy The weight strategy
       */
      static std::unique_ptr<Node> createROOT(
          const RNode& rnode,
          std::unique_ptr<IBranchNamer>&& namer,
          bool isMC,
          const std::string& name = "ROOT",
          const std::string& cutflowName = "Number of events",
          const std::string& weight = "",
          WeightStrategy strategy = WeightStrategy::Default)
      {
        return std::unique_ptr<Node>(
            new Node(rnode, std::move(namer), isMC, 
              name, cutflowName, weight, strategy) );
      }

      /**
       * @brief Trigger the run
       * @param printEvery How often to print to the screen
       */
      void run(ULong64_t printEvery);

      /**
       * @brief Trigger the run
       * @param printEvery How often to print to the screen
       * @param total The total number of events
       *
       * This is different to the function above in that it prints as a fraction
       * of the total.
       */
      void run(ULong64_t printEvery, ULong64_t total);

      /**
       * @brief Trigger the run
       * @tparam Monitor The monitor type
       * @param monitor The monitor
       *
       * Trigger the run, calling the operator()(unsigned int) of the monitor.
       * The argument to this call is the slot number. If you are not running in
       * a multi-threaded environment this will always be 0.
       * You are responsible for the thread safety of this function!
       */
      template <typename Monitor>
        void run(Monitor monitor);

    private:
      /**
       * @brief Create the root node of the tree
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param weight Expression to calculate a weight.
       * @param strategy Weighting strategy for this weight
       */
      Node(
          const RNode& rnode,
          std::unique_ptr<IBranchNamer>&& namer,
          bool isMC,
          const std::string& name = "ROOT",
          const std::string& cutflowName = "Number of events",
          const std::string& weight = "",
          WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Create the root node of the tree
       * @tparam W The functor used to calculate the weight
       * @param rnode The RDataFrame that forms the base of the tree
       * @param namer The branch namer
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param w Functor used to calculate the weight
       * @param columns The input columns for the weight
       * @param strategy Weighting strategy for this weight
       */
      template <typename W>
        Node(
            const RNode& rnode,
            std::unique_ptr<IBranchNamer>&& namer,
            bool isMC,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& columns,
            WeightStrategy strategy);

      /**
       * @brief Create a child node
       * @param parent The parent of this node
       * @param rnodes This node's RNodes
       * @param name The name of this node
       * @param cutflowName How this node appears in cutflows
       * @param weight Expression to calculate the node weight
       * @param strategy Weighting strategy for this weight
       */
      Node(
          Node& parent,
          std::map<std::string, RNode>&& rnodes,
          const std::string& name,
          const std::string& cutflowName,
          const std::string& weight,
          WeightStrategy strategy);


      /**
       * @brief Create a child node
       * @tparam W The functor used to calculate the weight
       * @param parent The parent of this node
       * @param rnodes This node's RNodes
       * @param name The name of this node
       * @param cutflowName How this node appears in cutflows
       * @param w Functor used to calculate the weight
       * @param columns The input columns for the weight
       * @param strategy Weighting strategy for this weight
       */
      template <typename W>
        Node(
            Node& parent,
            std::map<std::string, RNode>&& rnodes,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& columns,
            WeightStrategy strategy);

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

#ifndef RDFAnalysis_NodeBase_H
#define RDFAnalysis_NodeBase_H

// Package includes
#include "RDFAnalysis/IBranchNamer.h"
#include "RDFAnalysis/Helpers.h"
#include "RDFAnalysis/SysResultPtr.h"
#include "RDFAnalysis/SysVar.h"
#include "RDFAnalysis/WeightStrategy.h"

// ROOT includes
#include "ROOT/RDataFrame.hxx"
#include <TObject.h>

// STL includes
#include <string>
#include <map>
#include <memory>
#include <array>

/**
 * @file NodeBase.h
 * @brief The base class for the \ref Node classes.
 */

namespace RDFAnalysis {
  using RNode = ROOT::RDF::RNode;
  using ColumnNames_t = ROOT::RDataFrame::ColumnNames_t;

  /**
   * @brief Base class for the \ref Node classes.
   *
   * This class contains everything that does not depend on the \ref Detail
   * parameter of the Node class. Many functions merely forward their calls onto
   * the underlying ROOT::RNode objects, performing the necessary steps to allow
   * for weights and systematics so you should consult the RDataFrame
   * documentation for explanations of those functions.
   */
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
       * @brief Define several new variables on this node in a single statement.
       * @tparam F The functor type
       * @tparam Ret_t The return type of the functor - must be a tuple
       * @tparam N The number of defined arguments
       * @param names The names of the defined variables
       * @param f The functor
       * @param columns  The inputs to the functor
       * @return a non-owning pointer to this object.
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
        std::enable_if_t<N==std::tuple_size<Ret_t>::value, NodeBase*> Define(
            const std::array<std::string, N>& names,
            F f,
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
       * @param weight The column containing the weight information
       * @param WeightStrategy strategy The weight strategy to use
       *
       * Note that right now this won't work if T doesn't inherit from TH1. TODO
       * fix this! 
       */
      template <typename T>
        SysResultPtr<T> Fill(
            const T& model,
            const ColumnNames_t& columns,
            const std::string& weight = "",
            WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Execute a user-defined accumulation function.
       * @tparam AccFun The type of the accumulation function
       * @tparam MergeFun The type of the merging function
       * @tparam U The return type of the accumulation function
       *
       * @param aggregator The accumulation function
       * @param merger The merging function
       * @param columnName The column for the funtion to act on
       * @param aggIdentity The identity
       * @see https://root.cern.ch/doc/v614/classROOT_1_1RDF_1_1RInterface.html#ae540b00addc441f9b504cbae0ef0a24d
       */
      template <typename AccFun, typename MergeFun,
               typename ArgTypes=typename ROOT::TTraits::CallableTraits<AccFun>::arg_types,
               typename U=ROOT::TTraits::TakeFirstParameter_t<ArgTypes>>
        SysResultPtr<U> Aggregate(
            AccFun aggregator,
            MergeFun merger,
            const std::string& columnName,
            const U& aggIdentity)
        {
          return ActResult<ROOT::RDF::RResultPtr<U>, AccFun, MergeFun, std::string_view, const U&>(
              &RNode::Aggregate,
              ColumnNames_t{columnName},
              aggregator, merger, SysVarBranch(columnName), aggIdentity);
        }

      /**
       * @brief Execute a user-defined accumulation function.
       * @tparam AccFun The type of the accumulation function
       * @tparam MergeFun The type of the merging function
       * @tparam U The return type of the accumulation function
       *
       * @param aggregator The accumulation function
       * @param merger The merging function
       * @param columnName The column for the funtion to act on
       * @see https://root.cern.ch/doc/v614/classROOT_1_1RDF_1_1RInterface.html#a7d82cb96013d9fe75cf4cc4a57f6d692
       */
      template <typename AccFun, typename MergeFun,
               typename ArgTypes=typename ROOT::TTraits::CallableTraits<AccFun>::arg_types,
               typename U=ROOT::TTraits::TakeFirstParameter_t<ArgTypes>>
        SysResultPtr<U> Aggregate(
            AccFun aggregator,
            MergeFun merger,
            const std::string& columnName)
        {
          return ActResult<ROOT::RDF::RResultPtr<U>, AccFun, MergeFun, std::string_view>(
              &RNode::Aggregate,
              ColumnNames_t{columnName},
              aggregator, merger, SysVarBranch(columnName) );
        }

      /**
       * @brief Count the number of entries processed by this node.
       * @see https://root.cern.ch/doc/v614/classROOT_1_1RDF_1_1RInterface.html#a37f9e00c2ece7f53fae50b740adc1456
       */
      SysResultPtr<ULong64_t> Count()
      {
        return ActResult<ROOT::RDF::RResultPtr<ULong64_t>>(
            &RNode::Count,
            ColumnNames_t{});
      }

      /**
       * @brief Transmit a systematically varied action to the underlying
       * ROOT::RNodes.
       *
       * @tparam TrArgs The types of the action's arguments
       * @tparam T The return type of the action
       * @tparam Args The types of the arguments before translation
       * @param f The action
       * @param columns The columns affected by the action
       * @param args The arguments to the action
       * @return A map of systematic name to action return type
       *
       * Most functions on the Node classes get routed through this or one of
       * its overloads. It carries out the following operations:
       *   -# Use \ref columns to determine which systematics affect this action
       *   -# Apply the action to each underlying systematic-specific RNode,
       *      even those not in the list found in the previous step
       *   -# For each remaining systematic from step 1 apply the action to the
       *      nominal RNode.
       * When applying an action for a specific systematic any arguments that
       * can be translated are. For information on argument translation see
       * SysVar.h
       *
       * The first parameter of \ref f should be a ROOT::RNode&, this will be
       * provided by this function and should not be included in \ref args.
       */
      template <typename... TrArgs, typename T, typename... Args>
        std::map<std::string, T> Act(
            std::function<T(RNode&, TrArgs...)> f,
            const ColumnNames_t& columns,
            Args&&... args);

      /**
       * @brief Transmit a systematically varied action to the underlying
       * ROOT::RNodes.
       *
       * @tparam F The function type
       * @tparam Args The argument types
       * @param f The action
       * @param columns The columns affected by the action
       * @param args The arguments to the action
       * @return A map of systematic name to action return type
       *
       * Overload for non-member functions, will forward the call to
       * Node::Act<TrArgs, T, Args>.
       */
      template <typename F, typename... Args,
               typename T=typename ROOT::TTraits::CallableTraits<F>::ret_type>
        std::map<std::string, T> Act(
            F&& f,
            const ColumnNames_t& columns,
            Args&&... args)
        {
          return Act(
              std::function<T(RNode&, typename sysvar_traits<Args&&>::value_type...)>(f),
              columns,
              std::forward<Args>(args)...);
        }

      /**
       * @brief Transmit a systematically varied action to the underlying
       * ROOT::RNodes.
       *
       * @tparam TrArgs The types of the action's arguments
       * @tparam T The return type of the action
       * @tparam Args The types of the arguments before translation
       * @param f The action
       * @param columns The columns affected by the action
       * @param args The arguments to the action
       * @return A map of systematic name to action return type
       *
       * Most functions on the Node classes get routed through this or one of
       * its overloads. It carries out the following operations:
       *   -# Use \ref columns to determine which systematics affect this action
       *   -# Apply the action to each underlying systematic-specific RNode,
       *      even those not in the list found in the previous step
       *   -# For each remaining systematic from step 1 apply the action to the
       *      nominal RNode.
       * When applying an action for a specific systematic any arguments that
       * can be translated are. For information on argument translation see
       * SysVar.h
       *
       * This overload is selected when \ref f is a member function of
       * ROOT::RNode. In this case is usually necessary to specify T and TrArgs
       * in the call.
       */
      template <typename T, typename... TrArgs, typename... Args>
        std::map<std::string, T> Act(
            T (RNode::*f)(TrArgs...),
            const ColumnNames_t& columns,
            Args&&... args);

      /**
       * @brief Specialised version of Node::Act for functions returning a
       * ROOT::RDF::RResultPtr
       *
       * @tparam TrArgs The types of the action's arguments
       * @tparam T The return type of the action, should be a
       * ROOT::RDF::RResultPtr
       * @tparam Args The types of the arguments before translation
       * @tparam U The type wrapped by T
       * @param f The action
       * @param columns The columns affected by the action
       * @param args The arguments to the action
       * @return The SysResultPtr generated by the action
       *
       * This function wraps Act for cases where it returns an RResultPtr and
       * wraps that return value in a SysResultPtr.
       */
      template <typename... TrArgs, typename T, typename... Args,
               typename U=typename T::Value_t>
        SysResultPtr<U> ActResult(
            std::function<T(RNode&, TrArgs...)> f,
            const ColumnNames_t& columns,
            Args&&... args)
        {
          return SysResultPtr<U>(
              namer().nominalName(),
              Act(f, columns, std::forward<Args>(args)...) );
        }

      /**
       * @brief Specialised version of Node::Act for functions returning a
       * ROOT::RDF::RResultPtr
       *
       * @tparam F The type of the action
       * @tparam T The return type of the action, should be a
       * ROOT::RDF::RResultPtr
       * @tparam Args The types of the arguments before translation
       * @tparam U The type wrapped by T
       * @param f The action
       * @param columns The columns affected by the action
       * @param args The arguments to the action
       * @return The SysResultPtr generated by the action
       *
       * This function wraps Act for cases where it returns an RResultPtr and
       * wraps that return value in a SysResultPtr.
       */
      template <typename F, typename... Args,
               typename T=typename ROOT::TTraits::CallableTraits<F>::ret_type,
               typename U=typename T::Value_t>
        SysResultPtr<U> ActResult(
            F&& f,
            const ColumnNames_t& columns,
            Args&&... args)
        {
          return ActResult(
              std::function<T(RNode&, typename sysvar_traits<Args&&>::value_type...)>(f),
              columns,
              std::forward<Args>(args)...);
        }

      /**
       * @brief Specialised version of Node::Act for functions returning a
       * ROOT::RDF::RResultPtr
       *
       * @tparam TrArgs The types of the action's arguments
       * @tparam T The return type of the action, should be a
       * ROOT::RDF::RResultPtr
       * @tparam Args The types of the arguments before translation
       * @tparam U The type wrapped by T
       * @param f The action
       * @param columns The columns affected by the action
       * @param args The arguments to the action
       * @return The SysResultPtr generated by the action
       *
       * This function wraps Act for cases where it returns an RResultPtr and
       * wraps that return value in a SysResultPtr. This overload is selected
       * when the action is a member function of ROOT::RNode.
       */
      template <typename T, typename... TrArgs, typename... Args,
                typename U=typename T::Value_t>
        SysResultPtr<U> ActResult(
            T (RNode::*f)(TrArgs...),
            const ColumnNames_t& columns,
            Args&&... args)
        {
          return SysResultPtr<U>(
              namer().nominalName(),
              Act(f, columns, std::forward<Args>(args)...) );
        }

      /// Get the name
      const std::string& name() const { return m_name; }

      /// Get the name in a cutflow
      const std::string& cutflowName() const { return m_cutflowName; }

      /// Is this anonymous?
      bool isAnonymous() const { return m_name.empty(); }

      /// Was 'MC' mode activated?
      bool isMC() const { return m_isMC; }

      /// Get the RNode objects
      const std::map<std::string, RNode>& rnodes() const { return m_rnodes; }
      /// Get the RNode objects
      std::map<std::string, RNode>& rnodes() { return m_rnodes; }

      /// The namer
      const IBranchNamer& namer() const { return *m_namer; }

      /// Iterate over the objects defined on this
      auto objects() { return as_range(m_objects); }
      /// (Const) iterate over all the objects defined on this
      auto objects() const { return as_range(m_objects); }

      /// Is the node the root?
      virtual bool isRoot() const = 0;

    protected:
      /// Helper struct that forces the initialisation of the branch namer.
      struct NamerInitialiser {
        NamerInitialiser() {} // no-op
        NamerInitialiser(
            IBranchNamer& namer,
            const std::map<std::string, ROOT::RDF::RNode>& rnodes) {
          namer.readBranchList(rnodes);
        }
      };

      /// Base case for unwinding multiple define calls
      template <std::size_t I, std::size_t N, typename... Elements>
        std::enable_if_t<I!=0, void> unwindDefine(
            const std::array<std::string, N>& names,
            const std::string& fullName,
            const std::tuple<Elements...>*);

      /// Unwind multiple define calls
      template <std::size_t I, std::size_t N, typename... Elements>
        std::enable_if_t<I == 0, void> unwindDefine(
            const std::array<std::string, N>& names,
            const std::string& fullName,
            const std::tuple<Elements...>*);


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
       * @param isMC If true, set the 'MC' mode, otherwise the 'data' mode
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param weight Expression to calculate a weight.
       * @param strategy Weighting strategy for this weight
       */
      NodeBase(
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
       * @param isMC If true, set the 'MC' mode, otherwise the 'data' mode
       * @param name The name of the root node
       * @param cutflowName How the root node appears in the cutflow
       * @param w Functor used to calculate the weight
       * @param columns The input columns for the weight
       * @param strategy Weighting strategy for this weight
       */
      template <typename W>
        NodeBase(
            const RNode& rnode,
            std::unique_ptr<IBranchNamer>&& namer,
            bool isMC,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& columns,
            WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Create a child node
       * @param parent The parent of this node
       * @param rnodes This node's RNodes
       * @param name The name of this node
       * @param cutflowName How this node appears in cutflows
       * @param weight Expression to calculate the node weight
       * @param strategy Weighting strategy for this weight
       */
      NodeBase(
          NodeBase& parent,
          std::map<std::string, RNode>&& rnodes,
          const std::string& name,
          const std::string& cutflowName,
          const std::string& weight,
          WeightStrategy strategy = WeightStrategy::Default);
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
        NodeBase(
            NodeBase& parent,
            std::map<std::string, RNode>&& rnodes,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& columns,
            WeightStrategy strategy = WeightStrategy::Default);

      /**
       * @brief Set the weight on this node
       * @tparam F the functor type
       * @param f The functor
       * @param columns The input columns to \ref f (if any)
       * @param parent The parent (if any) of this node
       * @param strategy The weighting strategy to apply
       * @return The name of the new weight
       * The new weight will be calculated and stored in a new branch.
       */
      template <typename F>
        enable_ifn_string_t<F, std::string> setWeight(
            F f,
            const ColumnNames_t& columns,
            NodeBase* parent,
            WeightStrategy strategy);

      /**
       * @brief Set the weight on this node
       * @param expression The expression to calculate the weight
       * @param parent The parent (if any) of this node
       * @param strategy The weighting strategy to apply
       * @return The name of the new weight
       * The new weight will be calculated and stored in a new branch.
       */
      std::string setWeight(
          const std::string& expression,
          NodeBase* parent,
          WeightStrategy strategy);

      /// Internal function to name the weight branch
      std::string nameWeight();

      /// The RNode objects, keyed by systematic
      std::map<std::string, RNode> m_rnodes;      

      /// The branch namer
      std::unique_ptr<IBranchNamer> m_namer;

      /// Helper struct to force early initialisation of the namer
      NamerInitialiser m_namerInit;

      /// Whether or not 'MC' mode was activated
      bool m_isMC;

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

      /* /// The node statistics */
      /* SysResultPtr<ULong64_t> m_stats; */

      /* /// The node statistics (including weights) */
      /* SysResultPtr<std::pair<float, float>> m_weightedStats; */
  }; //> end class NodeBase
} //> end namespace RDFAnalysis
#include "RDFAnalysis/NodeBase.icc"
#endif //> !RDFAnalysis_NodeBase_H

#ifndef RDFAnalysis_Scheduler_H
#define RDFAnalysis_Scheduler_H

// Package includes
#include "RDFAnalysis/Node.h"
#include "RDFAnalysis/SchedulerBase.h"
#include "RDFAnalysis/ScheduleNamer.h"
#include "RDFAnalysis/Auditors/IAuditor.h"

/**
 * @file Node.h
 * @brief File containing the analysis scheduler class.
 */

namespace RDFAnalysis {
  /**
   * @brief Job scheduler
   *
   * @tparam Detail The type of node detail to be scheduled
   *
   * The scheduler is designed to take care of ordering your Define and Filter
   * calls for you. Each such call has dependencies on both variables and
   * filters which the scheduler takes into account when it is called.
   */
  template <typename Detail>
    class Scheduler : public SchedulerBase {
      public:
        /// The node detail type
        using detail_t = Detail;
        /// The node type
        using node_t = Node<Detail>;
        /// The IAuditor type
        using auditor_t = IAuditor<Detail>;

        /**
         * @brief Create the scheduler from a node
         * @param root The root node to attach everything else to.
         */
        Scheduler(node_t* root) : m_root(root), m_namer(root->namer() ) {}

        /// Get the root node. After schedule has been called this will contain
        /// the whole data structure.
        node_t* root() { return m_root; }

        /**
         * @brief Schedule the analysis
         * @param graphFile If set, write the schedule to this file.
         */
        ScheduleNode& schedule(const std::string& graphFile = "");

        /// Helper struct to define a region
        struct Region {
          /// The node that defines the final selection of this region
          node_t* node;

          /// The TObject fills associated with this region.
          std::vector<SysResultPtr<TObject>> objects;
        }; //> end struct Region
        
        /**
         * @brief Get the string->region mapping.
         *
         * This map is filled by the schedule function so will not be valid
         * before this has been called.
         */
        std::map<std::string, Region>& regions() { return m_regions; }
        
        /**
         * @brief Get the string->region mapping.
         *
         * This map is filled by the schedule function so will not be valid
         * before this has been called.
         */
        const std::map<std::string, Region>& regions() const
        { return m_regions; }

        /// Add a new auditor
        void addAuditor(std::shared_ptr<auditor_t> auditor)
        { m_auditors.push_back(auditor); }

        /**
         * @brief Add a new auditor
         * @tparam A The auditor template type
         * @tparam Args The constructor argument types
         * @param args The constructor arguments
         *
         * This version uses the correct detail type to specialise the template
         */
        template <template <typename> class A, typename... Args>
          void addAuditor(Args&&... args)
          { m_auditors.push_back(std::make_shared<A<Detail>>(
                std::forward<Args>(args)...) ); }

        /// Get the auditors
        std::vector<std::shared_ptr<auditor_t>>& auditors()
        { return m_auditors; }

        /// (const) get the auditors
        const std::vector<std::shared_ptr<auditor_t>>& auditors() const
        { return m_auditors; }

        /**
         * @brief Register a new variable
         * @param name The name of the new variable
         * @param action The Define call to use
         * @param variables The variables that this depends on
         * @param filters The filters that this depends on
         * @param cost The estimate cost of this action
         */
        void registerVariableImpl(
            const std::string& name,
            std::function<void(node_t*)> action,
            const std::set<std::string>& variables = {},
            const std::set<std::string>& filters = {},
            float cost = 0);

        /**
         * @brief Register a new variable definition
         * @tparam F The functor type
         * @param name The name of the column to define
         * @param f The functor
         * @param columns The input variables (if any) to the functor
         * @param filters Any filters that this depends on
         * @param cost A cost estimate for this action
         */
        template <typename F>
          enable_ifn_string_t<F, void> registerVariable(
              const std::string& name, 
              F f,
              const ColumnNames_t& columns = {},
              const std::set<std::string>& filters = {},
              float cost = 0);

        /**
         * @brief Register a new variable definition
         * @param name The name of the column to define
         * @param expression The string expression to interpret
         * @param filters Any filters that this depends on
         * @param cost A cost estimate for this action
         */
        void registerVariable(
            const std::string& name,
            const std::string& expression,
            const std::set<std::string>& filters = {},
            float cost = 0);

        /**
         * @brief Register a new variable definition
         * @param name The name of the column to define
         * @param expression The string expression to interpret
         * @param columns The input variables (if any) to the functor
         * @param filters Any filters that this depends on
         * @param cost A cost estimate for this action
         */
        void registerVariable(
            const std::string& name,
            const std::string& expression,
            const ColumnNames_t& columns,
            const std::set<std::string>& filters,
            float cost = 0);


        /**
         * @brief Register a single action that defines multiple new variables
         * @tparam N The number of variables defined
         * @param names The names of the new variables
         * @param action The Define call to use
         * @param variables The variables that this depends on
         * @param filters The filters that this depends on
         * @param cost The estimate cost of this action
         */
        template <std::size_t N>
          void registerVariablesImpl(
              const std::array<std::string, N>& names,
              std::function<void(node_t*)> action,
              const std::set<std::string>& variables = {},
              const std::set<std::string>& filters = {},
              float cost = 0);

        /**
         * @brief Register a single action that defines multiple new variables
         * @tparam F The functor type
         * @tparam N The number of defined arguments
         * @param names The names of the defined variables
         * @param f The functor
         * @param columns  The inputs to the functor
         * @param filters The filters that this depends on
         * @param cost The estimate cost of this action
         */
        template <std::size_t N, typename F>
          enable_ifn_string_t<F, void> registerVariables(
              const std::array<std::string, N>& names,
              F f,
              const ColumnNames_t& columns,
              const std::set<std::string>& filters = {},
              float cost = 0);

        /**
         * @brief Register a new filter
         * @param name The name of the new filter
         * @param action The Define call to use
         * @param variables The variables that this depends on
         * @param filters The filters that this depends on
         * @param cost The estimated cost of this action
         *
         * Note that the name of this filter can be different from the name of
         * the Node object(s) it creates. This is to allow for dependencies
         * involving anonymous nodes.
         */
        void registerFilterImpl(
            const std::string& name,
            std::function<node_t*(node_t*)> action,
            const std::set<std::string>& variables = {},
            const std::set<std::string>& filters = {},
            float cost = 0);

      /**
       * @brief Register a new filter
       * @tparam F The functor type
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of both the new filter and the node it creates
       * @param cutflowName How the new node appears in the cutflow
       * @param strategy Weighting strategy for this weight
       * @param filters The filters that this depends on
       * @param cost The estimated cost of this action
       *
       * In this overload the functor calculates the pass decision and the
       * weight in one go, return std::make_tuple(pass, weight).
       */
      template <typename F>
        std::enable_if_t<std::is_convertible<typename ROOT::TTraits::CallableTraits<F>::ret_type, std::tuple<bool, float>>::value, void> registerFilter(
            F f,
            const ColumnNames_t& columns,
            const std::string& name,
            const std::string& cutflowName = "",
            WeightStrategy strategy = WeightStrategy::Default,
            const std::set<std::string>& filters = {},
            float cost = 0);

      /**
       * @brief Register a new filter
       * @tparam F The functor type
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of both the new filter and the node it creates
       * @param cutflowName How the new node appears in the cutflow
       * @param weight Expression to calculate the node weight
       * @param strategy Weighting strategy for this weight
       * @param filters The filters that this depends on 
       * @param cost The estimated cost of this action
       */
      template <typename F>
        std::enable_if_t<std::is_convertible<typename ROOT::TTraits::CallableTraits<F>::ret_type, bool>::value, void> registerFilter(
            F f,
            const ColumnNames_t& columns,
            const std::string& name,
            const std::string& cutflowName = "",
            const std::string& weight = "",
            WeightStrategy strategy = WeightStrategy::Default,
            const std::set<std::string>& filters = {},
            float cost = 0);

      /**
       * @brief Register a new filter
       * @param expression The expression to describe the filter
       * @param name The name of both the new filter and the node it creates
       * @param cutflowName How the new node appears in the cutflow
       * @param weight Expression to calculate the node weight
       * @param strategy Weighting strategy for this weight
       * @param filters The filters that this depends on
       * @param cost The estimated cost of this action
       */
      void registerFilter(
          const std::string& expression,
          const std::string& name,
          const std::string& cutflowName = "",
          const std::string& weight = "",
          WeightStrategy strategy = WeightStrategy::Default,
          const std::set<std::string>& filters = {},
          float cost = 0);

      /**
       * @brief Register a new filter
       * @tparam F The functor type
       * @tparam W The functor type used for the weight
       * @param f The functor
       * @param columns The input variables to the functor
       * @param name The name of both the new filter and the node it creates
       * @param cutflowName How the new node appears in the cutflow
       * @param w The functor used to calculate the weight
       * @param weightColumns The input variables to the weight functor
       * @param strategy Weighting strategy for this weight
       * @param filters The filters that this depends on
       * @param cost The estimated cost of this action
       */
      template <typename F, typename W>
        std::enable_if_t<!std::is_convertible<F, std::string>::value && !std::is_convertible<W, std::string>::value, void> registerFilter(
            F f,
            const ColumnNames_t& columns,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& weightColumns = {},
            WeightStrategy strategy = WeightStrategy::Default,
            const std::set<std::string>& filters = {},
            float cost = 0);

      /**
       * @brief Register a new filter
       * @tparam W The functor type used for the weight
       * @param expression The expression to describe the filter
       * @param name The name of both the new filter and the node it creates
       * @param cutflowName How the new node appears in the cutflow
       * @param w The functor used to calculate the weight
       * @param weightColumns The input variables to the weight functor
       * @param strategy Weighting strategy for this weight
       * @param filters The filters that this depends on
       * @param cost The estimated cost of this action
       */
      template <typename W>
        enable_ifn_string_t<W, void> registerFilter(
            const std::string& expression,
            const std::string& name,
            const std::string& cutflowName,
            W w,
            const ColumnNames_t& weightColumns = {},
            WeightStrategy strategy = WeightStrategy::Default,
            const std::set<std::string>& filters = {},
            float cost = 0);
        
        /**
         * @brief Register a new fill
         * @param name The name of the new fill
         * @param action The Fill call ot use
         * @param variables THe variables that this depends on
         * @param filters The filters that this depends on
         *
         * Fills always get cost == 0, the ordering of fills is completely
         * unimportant
         */
        void registerFillImpl(
            const std::string& name,
            std::function<SysResultPtr<TObject>(node_t*)> action,
            const std::set<std::string>& variables = {},
            const std::set<std::string>& filters = {});

      /**
       * @brief register a new fill
       * @tparam T The type of object to be filled.
       * @param model The 'model' object to fill.
       * @param columns The columns to use for the object's Fill method.
       * @param weight The column containing the weight information
       * @param strategy The weight strategy to use
       * @param filters The filters that this depends on
       *
       * Note that right now this won't work if T doesn't inherit from TH1. TODO
       * fix this! 
       */
      template <typename T>
        void registerFill(
            const T& model,
            const ColumnNames_t& columns,
            const std::string& weight = "",
            WeightStrategy strategy = WeightStrategy::Default,
            const std::set<std::string>& filters = {});

      protected:
        /// The defined filters
        std::map<std::string, std::function<node_t*(node_t*)>> m_filters;
        /// The defined variables
        std::map<std::string, std::function<void(node_t*)>> m_variables;
        /// The defined fills
        std::map<std::string, std::function<SysResultPtr<TObject>(node_t*)>> m_fills;
        /// The root node
        node_t* m_root;
        /// The namer
        ScheduleNamer m_namer;
        /// After scheduling, pointers to the end nodes for all defined regions
        /// will be here
        std::map<std::string, Region> m_regions;
        /// The auditors to apply
        std::vector<std::shared_ptr<auditor_t>> m_auditors;
        /// Copy information across from the Schedule node to the actual node
        void addNode(const ScheduleNode& source, 
                     node_t* target,
                     const std::string& currentRegion = "");
    }; //> end class Scheduler
} //> end namespace RDFAnalysis
#include "RDFAnalysis/Scheduler.icc"
#endif //> !RDFAnalysis_Scheduler_H

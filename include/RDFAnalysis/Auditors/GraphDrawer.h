#ifndef RDFAnalysis_Auditors_GraphDrawer_H
#define RDFAnalysis_Auditors_GraphDrawer_H

#include "RDFAnalysis/Auditors/IAuditor.h"

namespace RDFAnalysis {
  class GraphDrawerBase {
    public:
      /// Store properties for the output graph and their defaults here.
      struct Properties {
        /// Whether to draw nodes that define variables
        bool writeVariables = true;
        /// Whether to draw nodes that fill histograms
        bool writeFills = true;
        /// The shape to use for filters
        std::string filterShape = "diamond";
        /// The shape to use for variables
        std::string variableShape = "oval";
        /// The shape to use for fills
        std::string fillShape = "box";
      }; //> end struct Properties

      /**
       * @brief Set up the drawer
       * @param fileName The name of the file to write to
       * @param properties The properties of the output graph
       */
      GraphDrawerBase(
          const std::string& fileName,
          const Properties& properties);

      /// Give access to the properties object
      Properties& properties() { return m_properties; }
      /// Give (const) access to the properties object
      const Properties& properties() const { return m_properties; }

    protected:
      void printSchedule(const SchedulerBase::ScheduleNode& source);
      /// The output file name
      std::string m_fileName;
      /// The output graph's properties
      Properties m_properties;

  }; //> end class GraphDrawerBase
  template <typename Detail> class GraphDrawer : public IAuditor<Detail>,
                                                 public GraphDrawerBase
  {
    public:
      using node_t = typename IAuditor<Detail>::node_t;


      /**
       * @brief Set up the drawer
       * @param fileName The name of the file to write to
       * @param properties The properties of the output graph
       */
      GraphDrawer(
          const std::string& fileName = "graph.dot",
          const Properties& properties = {});

      ~GraphDrawer() override {}

      /// Draw the graph and write it to file
      void auditSchedule(const SchedulerBase::ScheduleNode& source) override;
  }; //> end class GraphDrawer<Detail>
} //> end namespace RDFAnalysis

#include "RDFAnalysis/Auditors/GraphDrawer.icc"

#endif //> !RDFAnalysis_Auditors_GraphDrawer_H

#include "RDFAnalysis/Auditors/GraphDrawer.h"
#include "RDFAnalysis/Utils/BoostGraphBuilder.h"
#include <boost/graph/graphviz.hpp>
#include <fstream>

namespace {
  using namespace RDFAnalysis;
  using vertex_t = std::pair<SchedulerBase::ActionType, std::string>;
  class ActionGraphBuilder : public detail::BoostGraphBuilder<
                             const SchedulerBase::ScheduleNode&,
                             vertex_t>
  {
    public:
      using base_t = detail::BoostGraphBuilder<const SchedulerBase::ScheduleNode&, vertex_t>;
      using input_node_t = typename base_t::input_node_t;
      using vertex_info_t = typename base_t::vertex_info_t;
      using graph_t = typename base_t::graph_t;
      using vertex_t = typename base_t::Vertex;
      ActionGraphBuilder(bool writeVariables,
                         bool writeFills) :
        base_t([] (input_node_t input) { return input.children.begin(); },
            [] (input_node_t input) { return input.children.end(); }),
        m_writeVariables(writeVariables),
        m_writeFills(writeFills) {}

    private:
      vertex_info_t info(input_node_t input)
      {
        return std::make_pair(input.action.type, input.action.name);
      }

      typename base_t::NodeDecision processNode(input_node_t input)
      {
        switch(input.action.type) {
          case SchedulerBase::FILTER:
            return NodeDecision::Write;
          case SchedulerBase::VARIABLE:
            return m_writeVariables ? NodeDecision::Write : NodeDecision::Skip;
          case SchedulerBase::FILL:
            return m_writeFills ? NodeDecision::Write : NodeDecision::Skip;
          default:
            return NodeDecision::Skip;
        }
      }

      bool m_writeVariables;
      bool m_writeFills;
  };
  using prop_map_t = typename ActionGraphBuilder::base_t::prop_map_t;

  /* ActionGraphBuilder::graph_t graph = ActionGraphBuilder( */
  /*     m_properties.writeVariables, */
  /*     m_properties.writeFills).buildGraph(source); */
  class ActionWriter {
    public:
      ActionWriter(
          const prop_map_t& propMap,
          const std::string& filterShape,
          const std::string& variableShape,
          const std::string& fillShape) : 
        m_propMap(propMap),
        m_filterShape(filterShape),
        m_variableShape(variableShape),
        m_fillShape(fillShape) {}

      template <class VertexOrEdge>
        void operator()(std::ostream& out, const VertexOrEdge& v) const {
          const auto& prop = m_propMap[v];
          std::string shape;
          switch(prop.first) {
            case SchedulerBase::FILTER:
              shape = m_filterShape;
              break;
            case SchedulerBase::VARIABLE:
              shape = m_variableShape;
              break;
            default:
              shape = m_fillShape;
          }
          out << "[label=\"" << prop.second << "\" shape=" << shape << "]";
        }
    private:
      ActionGraphBuilder::prop_map_t m_propMap;
      std::string m_filterShape;
      std::string m_variableShape;
      std::string m_fillShape;
  };
} //> end anonymous namespace

namespace RDFAnalysis {

  GraphDrawerBase::GraphDrawerBase(
      const std::string& fileName,
      const Properties& properties) :
    m_fileName(fileName),
    m_properties(properties) {}

  void GraphDrawerBase::printSchedule(const SchedulerBase::ScheduleNode& source)
  {
  std::ofstream fs(m_fileName);
  if (!fs.is_open() )
    throw std::runtime_error("Failed to open '" + m_fileName +"'");
  ActionGraphBuilder::graph_t graph = ActionGraphBuilder(
      m_properties.writeVariables,
      m_properties.writeFills).buildGraph(source);
  prop_map_t propMap = boost::get(&ActionGraphBuilder::vertex_t::info, graph);
  boost::write_graphviz(fs, graph, ActionWriter(
        propMap,
        m_properties.filterShape,
        m_properties.variableShape,
        m_properties.fillShape) );
  }
} //> end namespace RDFAnalysis

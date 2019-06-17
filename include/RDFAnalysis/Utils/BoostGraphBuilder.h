#ifndef RDFAnalysis_Utils_BoostGraphBuilder_H
#define RDFAnalysis_Utils_BoostGraphBuilder_H

#include <boost/graph/adjacency_list.hpp>

#include <vector>
#include <functional>
#include <type_traits>

/**
 * @file BoostGraphBuilder.h
 * @brief File containing helper classes to build BGL graphs
 */

namespace RDFAnalysis { namespace detail {
  /**
   * @brief Class to build Boost Graph Library graphs from a recursive tree
   * structure
   * @tparam InputNode The type of the nodes in the tree
   * @tparam VertexInfo The type of info to attach to each vertex. Must be
   * default constructible and copy constructible
   * @tparam ChildItr The type of iterator used to iterate over child nodes
   *
   * Right now this class only allows for customising the vertices of the ouptut
   * graph. If necessary it could be extended to write extra information to the
   * edges.
   */
  template <typename InputNode,
            typename VertexInfo,
            typename ChildItr=typename std::vector<std::decay_t<InputNode>>::const_iterator>
    class BoostGraphBuilder {
      public:
        /// The vertex struct
        struct Vertex {
          /// Default construct the vertex
          Vertex() {}
          /// Construct the vertex from its info
          Vertex(const VertexInfo& info) : info(info) {}
          /// The information attached to a vertex
          VertexInfo info;
        }; //> end struct Vertex
        // Type defs
        /// The input node type
        using input_node_t = InputNode;
        /// The vertex info type
        using vertex_info_t = VertexInfo;
        /// The child iterator type
        using child_itr_t = ChildItr;
        /// The BGL graph type
        using graph_t = boost::adjacency_list<
          boost::vecS, boost::vecS, boost::directedS, Vertex>;
        /// The BGL vertex type
        using vert_desc_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
        /// The BGL property map type
        using prop_map_t = typename boost::property_map<graph_t, vertex_info_t Vertex::*>::type;

        /**
         * @brief Create the builder
         * @param childBegin Function to get the start of a node's children
         * @param childEnd Function to get the end of a node's children
         */
        BoostGraphBuilder(
            std::function<ChildItr(InputNode)> childBegin,
            std::function<ChildItr(InputNode)> childEnd);

        virtual ~BoostGraphBuilder() = 0;

        /**
         * @brief Build the graph
         * @param root The root node of the input tree.
         */
        graph_t buildGraph(InputNode root);

      protected:
        /// Create a vertex info from an input node
        virtual VertexInfo info(InputNode input) = 0;

        /// Enum to describe what should be done with a node
        enum class NodeDecision {
          Write,
          Skip,
          Terminate
        };

        /// Choose whether or not to add a node to a graph
        virtual NodeDecision processNode(InputNode input);

        /// Add a node into the graph
        void addToGraph(
            InputNode input,
            const vert_desc_t& parent,
            graph_t& graph);

        /// Get the start of a node's children
        std::function<ChildItr(InputNode)> m_childBegin;
        /// Get the end of a node's children
        std::function<ChildItr(InputNode)> m_childEnd;

    }; //> end class BoostGraphBuilder<InputNode, ChildBegin, ChildEnd, VertexInfo>
} } //> end namespace RDFAnalysis::detail
#include "RDFAnalysis/Utils/BoostGraphBuilder.icc"

#endif //> !RDFAnalysis_Utils_BoostGraphBuilder_H

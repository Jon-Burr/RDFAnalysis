#ifndef RDFAnalysis_INodeWriter_H
#define RDFAnalysis_INodeWriter_H

#include "RDFAnalysis/Node.h"

#include <string>

class TDirectory;

namespace RDFAnalysis {
  /**
   * @brief Base class for writing specific information from Nodes to file.
   *
   * @tparam Detail The templated Detail type of the \ref Node class to read
   * from.
   *
   * This class provides for a uniform interface for objects that extract
   * information from a given node to write to an output file. This allows
   * detailed and flexible configuration of what is written to the output.
   */
  template <typename Detail>
    class INodeWriter {
      public:
        virtual ~INodeWriter() {}

        /**
        * @brief Write the contents of \ref node to \ref directory.
        * @param node The node to write
        * @param directory The directory to write
        * @param depth How deep down the node structure we are.
        */
        virtual void write(
            Node<Detail>& node,
            TDirectory* directory,
            std::size_t depth) = 0;
    }; //> end class INodeWriter
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_INodeWriter_H

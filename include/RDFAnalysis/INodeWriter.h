#ifndef RDFAnalysis_INodeWriter_H
#define RDFAnalysis_INodeWriter_H

#include "RDFAnalysis/Node.h"

#include <string>

class TDirectory;

namespace RDFAnalysis {
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

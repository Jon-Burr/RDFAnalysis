// package includes
#include "RDFAnalysis/CutflowWriter.h"
#include "RDFAnalysis/Node.h"
#include "RDFAnalysis/Helpers.h"

#include <TDirectory.h>
#include <TH1.h>

namespace RDFAnalysis {
  CutflowWriter::CutflowWriter(const std::string& subDirName) :
    m_subDirName(subDirName)
  {}

  void CutflowWriter::write(
      Node& node,
      TDirectory* directory,
      std::size_t depth)
  {
    // If the size of the cutflow vector is larger than the depth then we have
    // finished one leg of the tree and should cut off the parts of the cutflow
    // relating to that leg
    if (m_cutflow.size() > depth)
      m_cutflow.resize(depth);
    // Add the cutflow information from this node
    auto result = node.stats();
    m_cutflow.emplace_back();
    for (auto& p : result.asMap() ) {
      m_cutflow.back()[p.first] = std::make_pair(node.cutflowName(), *p.second.get() );
      TH1F cutflowHist(p.first.c_str(), "Cutflow", m_cutflow.size(), 0, m_cutflow.size() );
      // Fill backwards
      for (std::size_t idx = m_cutflow.size(); idx > 0; --idx) {
        const auto& entry = getDefaultKey(
            m_cutflow[idx-1], p.first, node.namer().nominalName() );
        cutflowHist.SetBinContent(idx, entry.second);
        cutflowHist.SetBinError(idx, sqrt(entry.second) );
        cutflowHist.GetXaxis()->SetBinLabel(idx, entry.first.c_str() );
      }
      directory->WriteTObject(&cutflowHist);
    }
  }
} //> end namespace RDFAnalysis

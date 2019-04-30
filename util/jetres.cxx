#include "RDFAnalysis/Node.h"
#include <boost/program_options.hpp>
#include <TChain.h>
#include <iostream>
#include <TLorentzVector.h>
#include <memory>
#include <ROOT/RDFHelpers.hxx>
#include "RDFAnalysis/OutputWriter.h"
#include "RDFAnalysis/TObjectWriter.h"
#include "RDFAnalysis/DefaultBranchNamer.h"
#include <boost/algorithm/string/join.hpp>
#include <fstream>

namespace {
  void massPlots(
      std::shared_ptr<RDFAnalysis::Node> node,
      std::string jetStub,
      int nBins,
      float low,
      float high,
      float pTThreshold,
      float dRThreshold = 0.2)
  {
    auto filter = node->Filter(
        jetStub+"Pt > " +std::to_string(pTThreshold) + " && " + jetStub+"DRBoson < " + std::to_string(dRThreshold),
        jetStub+"Selection", jetStub+"Selection");
    filter->Fill(TH1F("JetMass", "m_{J} [MeV]", nBins, low, high), {jetStub+"Mass"});
  }
}
int main(int argc, char* argv[]) {
  std::string treeName;
  Long64_t nEvents;
  std::string output;
  bool overwrite{false};
  std::vector<std::string> inputFiles;
  std::string inTxt;
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("treeName,t", po::value(&treeName)->default_value("AnalysisTree"),
     "The name of the tree")
    ("nEvents,n", po::value(&nEvents)->default_value(-1), "The number of events over which to run. -1 will run all available events")
    ("output,o", po::value(&output)->default_value("hist-out.root"), "The output ROOT file")
    ("overwrite,w", po::bool_switch(&overwrite), "Enable overwriting of the output file")
    ("inTxt,I", po::value(&inTxt), "Text file containing inputs")
    ("help,h", "Print this message and exit");
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("inputFiles", po::value(&inputFiles) );
  po::positional_options_description positional;
  positional.add("inputFiles", -1);
  po::variables_map vm;
  po::options_description allOptions;
  allOptions.add(desc).add(hidden);
  po::store(po::command_line_parser(argc, argv).options(allOptions).positional(positional).run(), vm);
  po::notify(vm);
  if (vm.count("help") ) {
    std::cout << desc << std::endl;
    return 0;
  }
  if (!inTxt.empty() ) {
    std::string line;
    std::ifstream inTxtFin(inTxt);
    if (!inTxtFin.is_open() ) {
      std::cerr << "Failed to open input text file " << inTxt << std::endl;
      return 1;
    }
    while (std::getline(inTxtFin, line) )
      if (!line.empty() )
        inputFiles.push_back(line);
  }
  TChain input(treeName.c_str() );
  for (const std::string& fin : inputFiles)
    if (input.Add(fin.c_str(), -1) == 0) {
      std::cerr << "Tree " << treeName << " not found in file " << fin << std::endl;
      return 1;
    }

  auto root = RDFAnalysis::createROOT(
      ROOT::RDataFrame(input),
      std::make_unique<RDFAnalysis::DefaultBranchNamer>(std::vector<std::string>{""}, true, false, "") );

  auto Z = root->Filter("BosonID == 23", "Z", "Z");
  auto Zbottom = Z->Filter("BosonDecayID == 5", "Zbottom", "Zbottom");
  auto Zcharm = Z->Filter("BosonDecayID == 4", "Zcharm", "Zcharm");
  auto Zlight = Z->Filter("BosonDecayID < 4", "Zlight", "Zlight");
  auto H = root->Filter("BosonID == 25", "Higgs", "Higgs");
  for (const std::string& jet : {"FatJet", "FatJetCalib", "FatJetTruth"}) {
    for (const std::shared_ptr<RDFAnalysis::Node>& zn : {Zbottom, Zcharm, Zlight}) {
      massPlots(zn, jet, 70, 50e3, 120e3, 200e3);
    }
    massPlots(H, jet, 70, 80e3, 150e3, 250e3);
  }
  RDFAnalysis::OutputWriter outputWriter(output, overwrite);
  outputWriter.addWriter(RDFAnalysis::TObjectWriter() );
  std::cout << "Trigger run" << std::endl;
  root->rootRNode().Foreach(
      [nEvents = input.GetEntries()] (ULong64_t entry) {
        if (entry%5000 == 0) 
          std::cout << entry << "/" << nEvents << std::endl;
      }, {"rdfentry_"});
  outputWriter.write(*root);

  return 0;
}

#include "RDFAnalysis/Node.h"
#include <boost/program_options.hpp>
#include <TTree.h>
#include <TFile.h>
#include <TKey.h>
#include <iostream>
#include <TLorentzVector.h>
#include <memory>
#include <ROOT/RDFHelpers.hxx>
#include "RDFAnalysis/OutputWriter.h"
#include "RDFAnalysis/TObjectWriter.h"
#include "RDFAnalysis/DefaultBranchNamer.h"
#include <boost/algorithm/string/join.hpp>

int main(int argc, char* argv[]) {
  std::string nominalTree;
  Long64_t nEvents;
  std::string output;
  bool overwrite{false};
  std::string inputFile;
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("inputFile,i", po::value(&inputFile), "The input file")
    ("nominalTree,t", po::value(&nominalTree)->default_value("NOSYS"), "The name of the nominal tree (and path to it if necessary) within the input ROOT files")
    ("nEvents,n", po::value(&nEvents)->default_value(-1), "The number of events over which to run. -1 will run all available events")
    ("output,o", po::value(&output)->default_value("hist-out.root"), "The output ROOT file")
    ("overwrite,w", po::bool_switch(&overwrite), "Enable overwriting of the output file")
    ("help,h", "Print this message and exit");
  po::variables_map vm;
  po::options_description allOptions;
  allOptions.add(desc);
  po::store(po::command_line_parser(argc, argv).options(allOptions).run(), vm);
  po::notify(vm);
  if (vm.count("help") ) {
    std::cout << desc << std::endl;
    return 0;
  }
  TFile fin(inputFile.c_str() );
  if (fin.IsZombie() )
    return 1;
  std::vector<TTree*> trees;
  std::vector<std::string> systematics;
  TTree* nominal = nullptr;
  for (TObject* ikey : *fin.GetListOfKeys() ) {
    TKey* key = dynamic_cast<TKey*>(ikey);
    if (!key) {
      std::cerr << "Failed to cast key " << ikey->GetName() << std::endl;
      return 1;
    }
    if (key->GetClassName() == std::string("TTree") ) {
      trees.emplace_back((TTree*)key->ReadObj() );
      systematics.push_back(key->GetName() );
      if (key->GetName() == nominalTree)
        nominal = trees.back();
    }
  }
  if (!nominal) {
    std::cerr << "Nominal tree " << nominalTree << " not found in file "
      << inputFile << std::endl;
    return 1;
  }
  for (TTree* tree : trees) {
    if (tree == nominal)
      continue;
    nominal->AddFriend(tree);
  }
  auto root = RDFAnalysis::createROOT(
      ROOT::RDataFrame(*nominal), 
      std::make_unique<RDFAnalysis::DefaultBranchNamer>(systematics) );
  root->Define(
        "SelectedJets",
        [] (
          const std::vector<float>& jetPt,
          const std::vector<float>& jetEta,
          const std::vector<float>& jetPhi,
          const std::vector<float>& jetMass,
          const std::vector<char>& jetSel)
        {
          std::vector<TLorentzVector> out;
          for (std::size_t ii = 0; ii < jetPt.size(); ++ii) {
            if (jetSel.at(ii) ) {
              TLorentzVector vec;
              vec.SetPtEtaPhiM(jetPt.at(ii), jetEta.at(ii), jetPhi.at(ii), jetMass.at(ii));
              out.push_back(vec);
            }
          }
          return out;
        },
        {"JetPt", "JetEta", "JetPhi", "JetMass", "JetIsSignal"});

  auto twoJets = root->Filter(
      [] (const std::vector<TLorentzVector>& jets)
      { return jets.size() >=2; },
      { "SelectedJets" },
      "TwoJets",
      "nJets >= 2");

  /* twoJets->Histo1D( */
  /*     {"jetPt", ";p_{T} (j) [MeV]", 100, 0, 1e5}, */
  /*     "JetPt"); */

  const auto& namer = root->namer();

  auto expr = namer.expandExpression("JetPt[JetIsSignal]");
  std::cout << "JetPt[JetIsSignal] expands to :"<< std::endl;
  std::cout << expr.first << " (" << boost::algorithm::join(expr.second, ", ") << ")" << std::endl;
  std::cout << const_cast<RDFAnalysis::IBranchNamer&>(namer).interpretExpression(
      expr.first, expr.second, "JET_JER_EffectiveNP_1__1down");

  return 0;


  RDFAnalysis::OutputWriter outputWriter("test-hists.root", true);
  outputWriter.addWriter(RDFAnalysis::TObjectWriter() );
  std::cout << "Write" << std::endl;
  outputWriter.write(*root);


  /* const auto& namer = root->namer(); */
  /* std::cout << "Found systematics: " << std::endl; */
  /* for (const std::string& syst :  namer.systematics() ) */
  /*   std::cout << "\t" << syst << std::endl; */
  /* std::cout << "Found branches: " << std::endl; */
  /* for (const std::string& branchName : namer.branches() ) { */
  /*   std::cout << branchName << std::endl; */
  /*   for (const std::string& syst : namer.systematicsAffecting(branchName) ) */
  /*     std::cout << "\t" << syst << std::endl; */
  /* } */
  return 0;
}

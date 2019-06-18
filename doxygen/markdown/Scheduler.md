# The Job Scheduler # {#MD_JobScheduler}

- [Introduction](#Scheduler_Introduction)
- [Scheduler Concepts](#Scheduler_Concepts)
  - [Actions and dependencies](#Scheduler_Actions)
  - [Costs](#Scheduler_Costs)
  - [Auditors](#Scheduler_Auditors)
  - [Filter Satisfaction](#Scheduler_FilterSatisfaction)
- [Usage Example](#Scheduler_Usage)
- [Advanced Example](#Schedule_AdvancedExample)
- [Important notes and caveats](#Scheduler_Notes)

@section Scheduler_Introduction Introduction

Most users when designing an analysis do not generally care too much about the order in which variables are defined or how to merge the definitions of their various regions into a tree structure.
Instead, what you usually wish to have is a set of different regions (defined by selections on the dataset) and histograms (or similar) filled from within each of those regions.
The [Scheduler] is built around these assumptions.

Rather than explicitly building the tree structure through [Define] and [Filter] calls, and adding histograms through [Fill] calls, with the scheduler you instead tell it about all variables, filters and fills you want to use using the [registerVariable], [registerFilter] and [registerFill] functions.
Then, you define new regions using the [addRegion] function and add fills to those regions using the [addFill] function.
Finally, you schedule the full computational graph using the [schedule] function.

The output writer can [write from a Scheduler](@ref RDFAnalysis::OutputWriter::write(Scheduler<Detail>&)) directly.
When doing this, rather than the nested tree structure used before each region will be written to a separated directory.

The scheduling works in three phases.
First a 'raw' schedule is constructed that merges together all the region definitions into a single tree.
Then, the [Scheduler] calculates the necessary order in which to insert any [dependencies](#Scheduler_Actions) before each point in the raw schedule.
Finally, the scheduler performs the corresponding [Define], [Filter] and [Fill] calls following the calculated graph.

@section Scheduler_Concepts Scheduler Concepts
@subsection Scheduler_Actions Actions and dependencies

When building the computational graph the [Scheduler] considers all defined variables, filters and fills together.
We call these things 'Actions' (not to be confused with what ROOT::RDF::RNode terms Actions), represented by the RDFAnalysis::SchedulerBase::Action class.

Each action has a list of dependencies - these are the variables used as inputs to the operation and any selections necessary to make them valid.
For instance the calculation "m_ee = (Electrons.at(0)+Electrons.at(1)).M()" depends on the 'Electrons' variable and some selection that ensures that there are at least two electrons.
Each action has both direct dependencies like these but also indirect dependencies (each dependency's own direct dependencies).
The variable dependencies can usually be deduced from the structure of the call and filter dependencies are specified as extra argument at the end of the call.

When building the computational graph the scheduler will add all dependencies of a particular action in before it.
It will try to place filters as early as possible.
This allows easy creation of a histogram with preselections, just by listing the preselection as a filter dependency.

@subsection Scheduler_Costs Costs

Costs allow slightly more control over the order in which actions are scheduled.
Lower cost actions will be scheduled earlier than higher cost actions.
This can be used to fine-tune the output schedule.
Normally this will not be too necessary.

@subsection Scheduler_Auditors Auditors

Auditors allow direct inspection of the computations being performed.
They use the RDFAnalysis::IAuditor interface which allows inspecting the entire computational graph, and inserting extra operations before and after scheduled actions.
Currently two auditors are defined: The RDFAnalysis::GraphDrawer which creates a graphviz represention of the entire computational graph and the RDFAnalysis::DebugPrinter that inserts print outs to stdout before and after each action.
The [DebugPrinter](@ref RDFAnalysis::DebugPrinter) is useful because otherwise it can be extremely hard to pinpoint where in a computational graph a crash is coming from when just using RDFAnalysis::Node or ROOT::RDataFrame.

@subsection Scheduler_FilterSatisfaction Filter Satisfaction

One subtlety involved in job scheduling is that filters are often not independent - one filter can satisfy another.
For example, the selection x == 4 clearly satisfies the selection x > 2.
At best, scheduling x > 2 after x == 4 wastes CPU time.
At worst (when weights are involved) it can result in the double-counting of scale factors.

You can indicate to the scheduler that one filter satisfies another using the [filterSatisfies](@ref RDFAnalysis::Scheduler::filterSatisfies) function.
Note that these satisfaction relations are transitive: if A satisfies B and B satisfies C then the scheduler will work out that A also satisfies C.
More detail is provided in the [advanced example](#Schedule_AdvancedExample).

@section Scheduler_Usage Usage Example

Returning to the same example used before, this would be set up using the scheduler as

~~~{.cxx}
using node_t = RDFAnalysis::Node<CutflowDetail>;
using scheduler_t = RDFAnalysis::Scheduler<node_t::detail_t>;

std::unique_ptr<node_t> root = node_t::createROOT(
    inputDF, //> The input to this analysis
    std::make_unique<RDFAnalysis::DefaultBranchNamer>({"NOSYS"}), //> Systematics related
    false //> run without weights
    );
// Initialise the scheduler
scheduler_t scheduler(root.get() );
scheduler.addAuditor<RDFAnalysis::GraphDrawer>("graph.dot");

// Define new variables
// m_ee requires two electrons
scheduler.registerVariable("m_ee", "(Electrons.at(0) + Electrons.at(1)).M()", {"N_ELE_2"});
// m_mumu requires two muons
scheduler.registerVariable("m_mumu", "(Muons.at(0) + Muons.at(1)).M()", {"N_MU_2"});

// Define new filters
scheduler.registerFilter("Photons.size() == 1", "N_GAM_1", "Exactly one photon");
scheduler.registerFilter("Electrons.size() == 2", "N_ELE_2", "Exactly two electrons");
scheduler.registerFilter("Muons.size() == 2", "N_MU_2", "Exactly two muons");

// Define new fills
scheduler.registerFill(TH1F("m_ee", ";m_{ee} [GeV]", 50, 50, 100), {"m_ee"});
scheduler.registerFill(TH1F("m_mumu", ";m_{#mu#mu} [GeV]", 50, 50, 100), {"m_mumu"});

// Define the regions
using RegionDef = scheduler_t::RegionDef;
RegionDef& electronRegion = scheduler.addRegion("Electron", {"N_GAM_1", "N_ELE_2"});
electronRegion.addFill("m_ee");
RegionDef& muonRegion = scheduler.addRegion("Muon", {"N_GAM_1", "N_MU_2"});
muonRegion.addFill("m_mumu");

// Schedule the graph
scheduler.schedule();

RDFAnalysis::OutputWriter<node_t::detail_t> writer("output.root");
writer.addWriter<RDFAnalysis::CutflowWriter>();
writer.addWriter<RDFAnalysis::TObjectWriter>();
writer.write(scheduler);
~~~

This will produce exactly the same graph as in the previous example.

@section Schedule_AdvancedExample Advanced Example

In order to demonstrate some of the more advanced concepts than those that have already been seen we now consider attempting to reconstruct a H&rarr;bb decay.
We want to plot the kinematics of both b-jets (where they exist) as well as the mass of the bb pair - for all events as well as those with exactly two b-tagged jets.

Assuming an input dataset
~~~{.cxx}
std::vector<TLorentzVector> Jets; //> The jet kinematics
std::vector<char> IsBTagged; //> Whether or not an individual jet is b-tagged
std::vector<float> BTagSF; //> The scale factor for each jet
~~~

In order to make calculating the scale factors easier we define a few extra functions
~~~{.cxx}
// Require at least N b-tagged jets and return whether or not it passed and the
// corresponding scale factor
template <std::size_t N>
  std::tuple<bool, float> requireAtLeast(
      const ROOT::VecOps::RVec<char>& isBTagged,
      const ROOT::VecOps::RVec<float>& btagSFs)
{
  // Count until we reach N b-tagged jets
  std::size_t count;
  float sf = 1;
  for (std::size_t ii = 0; ii < isBTagged.size(); ++ii) {
    if (isBtagged.at(ii) )
      ++count;
    sf *= btagSFs.at(ii);
    if (count == N)
      break;
  }
  return std::make_tuple(count == N, sf);
}

// Require exactly N b-tagged jets and return whether or not it passed and the
// corresponding scale factor
template <std::size_t N>
  std::tuple<bool, float> requireExact(
      const ROOT::VecOps::RVec<char>& isBTagged,
      const ROOT::VecOps::RVec<float>& btagSFs)
{
  // Count until we reach N b-tagged jets
  std::size_t count;
  float sf = 1;
  for (std::size_t ii = 0; ii < isBTagged.size(); ++ii) {
    if (isBtagged.at(ii) )
      ++count;
    sf *= btagSFs.at(ii);
  }
  return std::make_tuple(count == N, sf);
}
~~~
Note that ROOT converts std::vectors to [ROOT::VecOps::RVec](https://root.cern/doc/master/classROOT_1_1VecOps_1_1RVec.html)s internally.

Then inside the main function, define the variables and schedule the regions as before
~~~{.cxx}
using node_t = RDFAnalysis::Node<CutflowDetail>;
using scheduler_t = RDFAnalysis::Scheduler<node_t::detail_t>;

std::unique_ptr<node_t> root = node_t::createROOT(
    inputDF, //> The input to this analysis
    std::make_unique<RDFAnalysis::DefaultBranchNamer>({"NOSYS"}), //> Systematics related
    false //> run without weights
    );
// Initialise the scheduler
scheduler_t scheduler(root.get() );
scheduler.addAuditor<RDFAnalysis::GraphDrawer>("graph.dot");

// Define the new variables
scheduler.registerVariable("BJets", "Jets[IsBTagged]"); //> Only the jets that are b-tagged
scheduler.registerVariable("BJet0", "BJets.at(0)", {"NB_GE_1"}); //> The leading b-jet kinematics
scheduler.registerVariable("BJet1", "BJets.at(1)", {"NB_GE_2"}); //> The sub-leading b-jet kinematics
// Define a lambda that retrieves the pt, eta, phi and m from a TLorentzVector
auto extractPtEtaPhiM = [] (const TLorentzVector& v) {
  return std::make_tuple(v.Pt(), v.Eta(), TVector2::Phi_mpi_pi(v.Phi()), v.M() );
}
scheduler.registerVariables<4>(
    {"BJet0Pt", "BJet0Eta", "BJet0Phi", "BJet0Mass"},
    extractPtEtaPhiM, {"BJet0"});
scheduler.registerVariables<4>(
    {"BJet1Pt", "BJet1Eta", "BJet1Phi", "BJet1Mass"},
    extractPtEtaPhiM, {"BJet1"});
scheduler.registerVariable("m_bb", "(BJet0+BJet1).M()");

// Define the filters
scheduler.registerFilter("NB_GE_1", requireAtLeast<1>, {"IsBTagged", "BTagSF"});
scheduler.registerFilter("NB_GE_2", requireAtLeast<2>, {"IsBTagged", "BTagSF"});
scheduler.registerFilter("NB_EQ_2", requireExact<2>, {"IsBTagged", "BTagSF"});
// Record the filter relations
scheduler.filterSatisfies("NB_GE_2", {"NB_GE_1"});
scheduler.filterSatisfies("NB_EQ_2", {"NB_GE_2"});

// Define the fills
scheduler.registerFill(TH1F("BJet0Pt", ";p_{T} (b-jet_{0}) [GeV]", 100, 0, 500), {"BJet0Pt"});
scheduler.registerFill(TH1F("BJet1Pt", ";p_{T} (b-jet_{1}) [GeV]", 100, 0, 500), {"BJet1Pt"});
scheduler.registerFill(TH1F("BJet0Eta", "#eta (b-jet_{0})", 10, -2.5, 2.5), {"BJet0Eta"});
scheduler.registerFill(TH1F("BJet1Eta", "#eta (b-jet_{1})", 10, -2.5, 2.5), {"BJet1Eta"});
scheduler.registerFill(TH1F("BJet0Phi", "#phi (b-jet_{0})", 70, -3.5, 3.5), {"BJet0Phi"});
scheduler.registerFill(TH1F("BJet1Phi", "#phi (b-jet_{1})", 70, -3.5, 3.5), {"BJet1Phi"});
scheduler.registerFill(TH1F("m_bb", ";m_{bb}", 75, 0, 150), {"m_bb"});

// Define the regions
using RegionDef = scheduler_t::RegionDef;
RegionDef& root = scheduler.addRegion("ROOT", {});
RegionDef& signalRegion = scheduler.addRegion("SignalRegion", {"NB_EQ_2"});
for (const std::string& h : {"BJet0Pt", "BJet1Pt", "BJet0Eta", "BJet1Eta", "BJet0Phi", "BJet1Phi", "m_bb"}) {
  root.addFill(h);
  signalRegion.addFill(h);
}

// Schedule the graph
scheduler.schedule();

RDFAnalysis::OutputWriter<node_t::detail_t> writer("output.root");
writer.addWriter<RDFAnalysis::CutflowWriter>();
writer.addWriter<RDFAnalysis::TObjectWriter>();
writer.write(scheduler);
~~~

This produces the following computational graph.
![](example_advanced_sel.png)

@section Scheduler_Notes Important notes and caveats

When using filters that involving scale factors (and therefore increase event-level weights) you need to be careful.
If, in the advanced example, we had not provided the `filterSatisfies` relations then to produce the signal region m_bb distribution all 3 filters ("NB_GE_1", "NB_GE_2" and "NB_EQ_2") would have been applied and scale factors up to and including the first b-jet would have been triple-counted and scale factors from there to the second b-jet double counted!
However, properly specifying these satisfaction relations fixes this problem.

Another thing to watch out for is the order in which actions are registered with the scheduler.
**With one exception** actions can be declared in any order, that exception being where new (i.e. not present in the input dataset) variables are used in string expressions.
[A specialised IBranchNamer class](@ref RDFAnalysis::ScheduleNamer) is used to extract variables from string expressions, but this only knows about variables after they have been registered.
This means that it's always better to register variables *first*, and to be careful with the ordering of those variables.

[Scheduler]: @ref RDFAnalysis::Scheduler
[Define]: @ref RDFAnalysis::Node::Define
[Filter]: @ref RDFAnalysis::Node::Filter
[Fill]: @ref RDFAnalysis::NodeBase::Fill
[registerVariable]: @ref RDFAnalysis::Scheduler::registerVariable
[registerFilter]: @ref RDFAnalysis::Scheduler::registerFilter
[registerFill]: @ref RDFAnalysis::Scheduler::registerFill
[addRegion]: @ref RDFAnalysis::SchedulerBase::addRegion
[addFill]: @ref RDFAnalysis::SchedulerBase::RegionDef::addFill
[schedule]: @ref RDFAnalysis::Scheduler::schedule

#ifndef RDFAnalysis_IBranchNamer_H
#define RDFAnalysis_IBranchNamer_H

// TODO documentation
// TODO const version of name branch that only gets you branches that exist

#include <string>
#include <vector>
#include <set>

namespace RDFAnalysis {
  class Node;
  class IBranchNamer {
    friend class Node;
    public:
      virtual ~IBranchNamer() {}

      virtual std::string nameBranch(
          const std::string& branch,
          const std::string& systName = "",
          bool create = false,
          bool isRNodeSys = false) = 0;

      virtual bool exists(
          const std::string& branch,
          const std::string& systName = "") const = 0;

      virtual const std::string& nominalName() const = 0;

      virtual std::vector<std::string> systematics() const = 0;

      virtual std::set<std::string> systematicsAffecting(
          const std::string& branch) const = 0;

      virtual std::set<std::string> systematicsAffecting(
          const std::vector<std::string>& branches) const;

      virtual std::vector<std::string> branches() const = 0;

      virtual void refresh() = 0;

      virtual std::unique_ptr<IBranchNamer> copy() const = 0;

      virtual std::pair<std::string, std::vector<std::string>> expandExpression(
          const std::string& expression) const;

      virtual std::string interpretExpression(
          const std::string& expression,
          const std::vector<std::string>& branches,
          const std::string& systematic);

    private:
      virtual void setNode(
          const Node& node,
          bool doRefresh = false) = 0;

  }; //> end class IBranchNamer
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_IBranchNamer_H

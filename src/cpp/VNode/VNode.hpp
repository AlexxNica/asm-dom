#ifndef VNode_hpp
#define VNode_hpp

#include <vector>
#include <string>
#include <map>

class VNode {
  public:
    VNode() {};
    VNode(
      const std::string nodeSel
    ): sel(nodeSel) {};
    VNode(
      const std::string nodeSel,
      const std::string nodeText
    ): sel(nodeSel), text(nodeText) {};
    VNode(
      const std::string nodeSel,
      const std::string nodeKey,
      const std::string nodeText,
      const std::map<std::string, std::string> nodeProps
    ): sel(nodeSel), key(nodeKey), text(nodeText), props(nodeProps) {};
    VNode(
      const std::string nodeSel,
      const std::string nodeKey,
      const std::string nodeText,
      const std::map<std::string, std::string> nodeProps,
      const std::vector<VNode*> nodeChildren
    ): sel(nodeSel), key(nodeKey), text(nodeText), props(nodeProps), children(nodeChildren) {};
    std::string sel; 
    std::string key;
    std::string text;
    std::map<std::string, std::string> props;
    int elm;
    std::vector<VNode*> children;
};

#endif

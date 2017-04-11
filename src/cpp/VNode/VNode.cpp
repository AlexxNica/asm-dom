#include "VNode.hpp"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <map>

VNode* makeVNodeWithSelAndText(
  const std::string nodeSel,
  const std::string nodeText
) {
  return new VNode(nodeSel, nodeText);
};

VNode* makeVNodeWithProps(
  const std::string nodeSel,
  const std::string nodeKey,
  const std::string nodeText,
  const std::map<std::string, std::string> nodeProps
) {
  return new VNode(nodeSel, nodeKey, nodeText, nodeProps);
};

VNode* makeVNode(
  const std::string nodeSel,
  const std::string nodeKey,
  const std::string nodeText,
  const std::map<std::string, std::string> nodeProps,
  const std::vector<VNode*> nodeChildren
) {
  return new VNode(nodeSel, nodeKey, nodeText, nodeProps, nodeChildren);
};

EMSCRIPTEN_BINDINGS(vnode) {
  emscripten::register_map<std::string, std::string>("MapStringString");
  emscripten::class_<VNode>("VNode")
    .constructor(&makeVNodeWithSelAndText, emscripten::allow_raw_pointers())
    .constructor(&makeVNodeWithProps, emscripten::allow_raw_pointers())
    .constructor(&makeVNode, emscripten::allow_raw_pointers());
  emscripten::register_vector<VNode*>("VNodeVector");
}

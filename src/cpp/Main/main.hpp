#ifndef Main_hpp
#define Main_hpp

#include "../VNode/VNode.hpp"
#include <emscripten/val.h>
#include <vector>

std::size_t patch_vnodePtr(const std::size_t oldVnode, const std::size_t vnode);
std::size_t patch_elementPtr(const emscripten::val element, const std::size_t vnode);
void updateChildren(
	emscripten::val parentElm,
	std::vector<VNode*> oldCh,
	std::vector<VNode*> newCh
);
void patchVnode(
	VNode* __restrict__ const oldVnode,
	VNode* __restrict__ const vnode
);

void deleteVNode(VNode* const vnode);
void deleteVNodePtr(const std::size_t& vnode);

#endif

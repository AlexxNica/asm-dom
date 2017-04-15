#include "main.hpp"
#include "../H/h.hpp"
#include "../Diff/diff.hpp"
#include "../VNode/VNode.hpp"
#include <emscripten.h>
#include <emscripten/bind.h>
#include <algorithm>
#include <vector>
#include <map>
#include <string>

VNode* const emptyNode = new VNode();

bool isDefined(const emscripten::val& obj) {
  std::string type = obj.typeOf().as<std::string>();
  return type.compare("undefined") != 0 && type.compare("null") != 0;
};

bool sameVnode(const VNode* __restrict__ const vnode1, const VNode* __restrict__ const vnode2) {
  return vnode1->key.compare(vnode2->key) == 0 && vnode1->sel.compare(vnode2->sel) == 0;
};

VNode* emptyNodeAt(const emscripten::val elm) {
  VNode* vnode = new VNode(elm["tagName"].as<std::string>());
	std::string id = elm["id"].as<std::string>();

  vnode->elm = EM_ASM_INT({
		return window['asmDomHelpers']['domApi']['addNode'](
			window['asmDomHelpers']['Pointer_stringify']($0)
		);
	}, id.c_str());
  std::transform(vnode->sel.begin(), vnode->sel.end(), vnode->sel.begin(), ::tolower);

  vnode->props.insert(
		std::make_pair(
			std::string("id"),
			id
		)
	);

  if (isDefined(elm["className"])) {
		vnode->props.insert(
			std::make_pair(
				std::string("class"),
				elm["className"].as<std::string>()
			)
		);
  }

  return vnode;
};

std::map<std::string, int>* createKeyToOldIdx(const std::vector<VNode*> children, const int beginIdx, const int endIdx) {
  std::size_t i = beginIdx;
	std::map<std::string, int>* map = new std::map<std::string, int>();
  for (; i <= endIdx; ++i) {
    if (!children[i]->key.empty()) {
			map->insert(std::make_pair(children[i]->key, i));
		}
  }
  return map;
}

int createElm(VNode* const vnode, const std::size_t& vnodePtr) {
	if (vnode->sel.compare("!") == 0) {
		vnode->elm = EM_ASM_INT({
			return window['asmDomHelpers']['domApi']['createComment'](
				window['asmDomHelpers']['Pointer_stringify']($0)
			);
		}, vnode->text.c_str());
	} else if (vnode->sel.empty()) {
		vnode->elm = EM_ASM_INT({
			return window['asmDomHelpers']['domApi']['createTextNode'](
				window['asmDomHelpers']['Pointer_stringify']($0)
			);
		}, vnode->text.c_str());
	} else {
		if (vnode->props.count(std::string("ns")) != 0) {
			vnode->elm = EM_ASM_INT({
				return window['asmDomHelpers']['domApi']['createElementNS'](
					window['asmDomHelpers']['Pointer_stringify']($0),
					window['asmDomHelpers']['Pointer_stringify']($1)
				);
			}, vnode->props.at(std::string("ns")).c_str(), vnode->sel.c_str());
		} else {
			vnode->elm = EM_ASM_INT({
				return window['asmDomHelpers']['domApi']['createElement'](
					window['asmDomHelpers']['Pointer_stringify']($0)
				);
			}, vnode->sel.c_str());
		}

		diff(emptyNode, vnode, vnodePtr);

		if (!vnode->children.empty()) {
			for(std::vector<VNode*>::size_type i = 0; i != vnode->children.size(); ++i) {
				EM_ASM_({
					window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
						'appendChild',
						$1,
						$2
					]);
				}, vnodePtr, vnode->elm, createElm(vnode->children[i], vnodePtr));
			}
		} else if (!vnode->text.empty()) {
			EM_ASM_({
				window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
					'appendChild',
					$1,
					window['asmDomHelpers']['domApi']['createTextNode']($2)
				]);
			}, vnodePtr, vnode->elm, vnode->text.c_str());
		}
	}
	return vnode->elm;
};

void addVnodes(
	int parentElm,
	int before,
	std::vector<VNode*> vnodes,
	std::vector<VNode*>::size_type startIdx,
	const std::vector<VNode*>::size_type endIdx,
	const std::size_t& vnodePtr
) {
	for (; startIdx <= endIdx; ++startIdx) {
		EM_ASM_({
			window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
				'insertBefore',
				$1,
				$2,
				$3 || null
			]);
		}, vnodePtr, parentElm, createElm(vnodes[startIdx], vnodePtr), before);
	}
};

void removeVnodes(
	std::vector<VNode*> vnodes,
	std::vector<VNode*>::size_type startIdx,
	const std::vector<VNode*>::size_type endIdx,
	const std::size_t& vnodePtr
) {
	for (; startIdx <= endIdx; ++startIdx) {
		EM_ASM_({
			window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
				'removeChild',
				$1
			]);
		}, vnodePtr, vnodes[startIdx]->elm);
	}
};

void updateChildren(
	int parentElm,
	std::vector<VNode*> oldCh,
	std::vector<VNode*> newCh,
	const std::size_t& vnodePtr
) {
	std::size_t oldStartIdx = 0;
	std::size_t newStartIdx = 0;
	std::size_t oldEndIdx = oldCh.size() - 1;
	std::size_t newEndIdx = newCh.size() - 1;
	VNode* oldStartVnode = oldCh[0];
	VNode* oldEndVnode = oldCh[oldEndIdx];
	VNode* newStartVnode = newCh[0];
	VNode* newEndVnode = newCh[newEndIdx];
	std::map<std::string, int>* oldKeyToIdx;
	VNode* elmToMove;

	while (oldStartIdx <= oldEndIdx && newStartIdx <= newEndIdx) {
		if (sameVnode(oldStartVnode, newStartVnode)) {
			patchVnode(oldStartVnode, newStartVnode, vnodePtr);
			oldStartVnode = oldCh[++oldStartIdx];
			newStartVnode = newCh[++newStartIdx];
		} else if (sameVnode(oldEndVnode, newEndVnode)) {
			patchVnode(oldEndVnode, newEndVnode, vnodePtr);
			oldEndVnode = oldCh[--oldEndIdx];
			newEndVnode = newCh[--newEndIdx];
		} else if (sameVnode(oldStartVnode, newEndVnode)) {
			patchVnode(oldStartVnode, newEndVnode, vnodePtr);
			EM_ASM_({
				window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
					'insertBefore',
					$1,
					$2,
					window['asmDomHelpers']['domApi']['nextSibling']($3)
				]);
			}, vnodePtr, parentElm, oldStartVnode->elm, oldEndVnode->elm);
			oldStartVnode = oldCh[++oldStartIdx];
			newEndVnode = newCh[--newEndIdx];
		} else if (sameVnode(oldEndVnode, newStartVnode)) {
			patchVnode(oldEndVnode, newStartVnode, vnodePtr);
			EM_ASM_({
				window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
					'insertBefore',
					$1,
					$2,
					$3
				]);
			}, vnodePtr, parentElm, oldEndVnode->elm, oldStartVnode->elm);
			oldEndVnode = oldCh[--oldEndIdx];
			newStartVnode = newCh[++newStartIdx];
		} else {
			if (!oldKeyToIdx) {
				oldKeyToIdx = createKeyToOldIdx(oldCh, oldStartIdx, oldEndIdx);
			}
			if (oldKeyToIdx->count(newStartVnode->key) == 0) {
				EM_ASM_({
					window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
						'insertBefore',
						$1,
						$2,
						$3
					]);
				}, vnodePtr, parentElm, createElm(newStartVnode, vnodePtr), oldStartVnode->elm);
				newStartVnode = newCh[++newStartIdx];
			} else {
				elmToMove = oldCh[oldKeyToIdx->at(newStartVnode->key)];
				if (elmToMove->sel.compare(newStartVnode->sel) != 0) {
					EM_ASM_({
						window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
							'insertBefore',
							$1,
							$2,
							$3
						]);
					}, vnodePtr, parentElm, createElm(newStartVnode, vnodePtr), oldStartVnode->elm);
				} else {
					patchVnode(elmToMove, newStartVnode, vnodePtr);
					deleteVNode(elmToMove);
					elmToMove = NULL;
					oldKeyToIdx->erase(newStartVnode->key);
					EM_ASM_({
						window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
							'insertBefore',
							$1,
							$2,
							$3
						]);
					}, vnodePtr, parentElm, elmToMove->elm, oldStartVnode->elm);
				}
				newStartVnode = newCh[++newStartIdx];
			}
		}
	}
	if (oldStartIdx > oldEndIdx) {
		if (newEndIdx + 1 <= newCh.size() - 1) {
			addVnodes(parentElm, newCh[newEndIdx+1]->elm, newCh, newStartIdx, newEndIdx, vnodePtr);
		} else {
			addVnodes(parentElm, 0, newCh, newStartIdx, newEndIdx, vnodePtr);
		}
	} else if (newStartIdx > newEndIdx) {
		removeVnodes(oldCh, oldStartIdx, oldEndIdx, vnodePtr);
	}
	delete oldKeyToIdx;
};

void patchVnode(VNode* __restrict__ const oldVnode, VNode* __restrict__ const vnode, const std::size_t& vnodePtr) {
	vnode->elm = oldVnode->elm;
	diff(oldVnode, vnode, vnodePtr);
	if (vnode->text.empty()) {
		if (!vnode->children.empty() && !oldVnode->children.empty()) {
			updateChildren(vnode->elm, oldVnode->children, vnode->children, vnodePtr);
		} else if(!vnode->children.empty()) {
			if (!oldVnode->text.empty()) {
				EM_ASM_({
					window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
						'setTextContent',
						$1,
						""
					]);
				}, vnodePtr, vnode->elm);
			};
			addVnodes(vnode->elm, 0, vnode->children, 0, vnode->children.size() - 1, vnodePtr);
		} else if(!oldVnode->children.empty()) {
			removeVnodes(oldVnode->children, 0, oldVnode->children.size() - 1, vnodePtr);
		} else if (!oldVnode->text.empty()) {
			EM_ASM_({
				window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
					'setTextContent',
					$1,
					""
				]);
			}, vnodePtr, vnode->elm);
		}
	} else if (vnode->text.compare(oldVnode->text) != 0) {
		EM_ASM_({
			window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
				'setTextContent',
				$1,
				window['asmDomHelpers']['Pointer_stringify']($2)
			]);
		}, vnodePtr, vnode->elm, vnode->text.c_str());
	}
};

VNode* patch_vnode(VNode* __restrict__ const oldVnode, VNode* __restrict__ const vnode, const std::size_t& vnodePtr) {
	if (sameVnode(oldVnode, vnode)) {
		patchVnode(oldVnode, vnode, vnodePtr);
	} else {
		int parent = EM_ASM_INT({
			return window['asmDomHelpers']['domApi']['parentNode']($0);
		}, oldVnode->elm);
		createElm(vnode, vnodePtr);
		if (parent) {
			EM_ASM_({
				window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
					'insertBefore',
					$1,
					$2,
					window['asmDomHelpers']['domApi']['nextSibling']($3)
				]);
			}, vnodePtr, parent, vnode->elm, oldVnode->elm);
			std::vector<VNode*> vnodes { oldVnode };
			removeVnodes(vnodes, 0, 0, vnodePtr);
		}
	}
	EM_ASM_({
		window['asmDomHelpers']['domApi']['commit']($0)
	}, vnodePtr);
	return vnode;
};

std::size_t patch_vnodePtr(const std::size_t oldVnode, const std::size_t vnode) {
	return reinterpret_cast<std::size_t>(patch_vnode(reinterpret_cast<VNode*>(oldVnode), reinterpret_cast<VNode*>(vnode), vnode));
};

std::size_t patch_elementPtr(const emscripten::val element, const std::size_t vnode) {
	return reinterpret_cast<std::size_t>(patch_vnode(emptyNodeAt(element), reinterpret_cast<VNode*>(vnode), vnode));
};

EMSCRIPTEN_BINDINGS(patch_function) {
	emscripten::function("patchVNode", &patch_vnodePtr, emscripten::allow_raw_pointers());
	emscripten::function("patchElement", &patch_elementPtr, emscripten::allow_raw_pointers());
}

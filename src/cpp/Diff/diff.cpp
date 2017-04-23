#include "diff.hpp"
#include "../VNode/VNode.hpp"
#include <emscripten.h>
#include <iterator>
#include <map>

void diff(VNode* __restrict__ const oldVnode, VNode* __restrict__ const vnode, const std::size_t& vnodePtr) {
	EM_ASM_({
		window['asmDomHelpers']['diff']($0, $1, $2);
	}, reinterpret_cast<std::size_t>(oldVnode), vnodePtr, vnode->elm);

	if (oldVnode->props.empty() && vnode->props.empty()) return;

	std::map<std::string, std::string>::iterator it = oldVnode->props.begin();
	while (it != oldVnode->props.end())
	{
		if (vnode->props.count(it->first) == 0) {
			EM_ASM_({
				window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
					'removeAttribute',
					$1,
					window['asmDom']['Pointer_stringify']($2)
				]);
			}, vnodePtr, vnode->elm, it->first.c_str());
		}
		++it;
	}

	it = vnode->props.begin();
	bool isAttrDefined;
	while (it != vnode->props.end()) {
		isAttrDefined = oldVnode->props.count(it->first) != 0;
		if (!isAttrDefined || (isAttrDefined && oldVnode->props.at(it->first).compare(it->second) != 0)) {
			EM_ASM_({
				window['asmDomHelpers']['vnodesData'][$0]['operations'].push([
					'setAttribute',
					$1,
					window['asmDom']['Pointer_stringify']($2),
					window['asmDom']['Pointer_stringify']($3)
				]);
			}, vnodePtr, vnode->elm, it->first.c_str(), it->second.c_str());
		}
		++it;
	}
};

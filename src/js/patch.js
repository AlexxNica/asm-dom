export default (oldVnode, vnode, options) => {
  let result;
  options = options || {};
  window.asmDomHelpers.vnodesData[vnode].callback = options.callback;
  if (typeof oldVnode === 'number') {
    result = window.asmDom.patchVNode(oldVnode, vnode);
    if (options.clearMemory === undefined || options.clearMemory) {
      setTimeout(() => {
        window.asmDom.deleteVNode(oldVnode);
      });
    }
  } else {
    result = window.asmDom.patchElement(oldVnode, vnode);
  }
  return result;
};

export const getPatchFunction = (lib) => (oldVnode, vnode) => {
  let result;
  if (oldVnode instanceof lib.VNode) {
    result = lib.patchVNode(oldVnode, vnode);
    lib.deleteVNode(oldVnode);
  } else {
    result = lib.patchElement(oldVnode, vnode);
  }
  return result;
};

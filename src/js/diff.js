export default (oldVnodePtr, vnodePtr, elmPtr) => {
  let oldListeners = (window.asmDomHelpers.vnodesData[oldVnodePtr] || {}).events;
  let newListeners = window.asmDomHelpers.vnodesData[vnodePtr].events;

  if (!oldListeners && !newListeners || oldListeners === newListeners) return;
  oldListeners = oldListeners || {};
  newListeners = newListeners || {};

  const operations = window.asmDomHelpers.vnodesData[vnodePtr].operations;

  for (const key in oldListeners) {
    if (!newListeners[key]) {
      operations.push(['setProp', elmPtr, key]);
    }
  }

  for (const key in newListeners) {
    if (oldListeners[key] !== newListeners[key]) {
      operations.push(['setProp', elmPtr, key, newListeners[key]]);
    }
  }

  operations.push(['setProp', elmPtr, 'asmDomListeners', Object.keys(newListeners)]);
};

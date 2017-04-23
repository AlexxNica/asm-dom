export default (oldVnodePtr, vnodePtr, elmPtr) => {
  let oldRaws = (window.asmDomHelpers.vnodesData[oldVnodePtr] || {}).raw;
  let newRaws = (window.asmDomHelpers.vnodesData[vnodePtr] || {}).raw;

  if (!oldRaws && !newRaws || oldRaws === newRaws) return;
  oldRaws = oldRaws || {};
  newRaws = newRaws || {};

  const operations = window.asmDomHelpers.vnodesData[vnodePtr].operations;

  for (const key in oldRaws) {
    if (!newRaws[key]) {
      operations.push(['setProp', elmPtr, key]);
    }
  }

  for (const key in newRaws) {
    if (oldRaws[key] !== newRaws[key]) {
      operations.push(['setProp', elmPtr, key, newRaws[key]]);
    }
  }

  operations.push(['setProp', elmPtr, 'asmDomRaws', Object.keys(newRaws)]);
};

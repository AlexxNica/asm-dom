import recycler from './domRecycler';

export const nodes = { 0: null };
let ptr = 0;

const addPtr = (node, ns) => {
  if (node) {
    node.asmDomPtr = ptr;
    node.asmDomNS = ns;
  }
  return node;
};

const domApi = {
  'addNode'(id) {
    const node = document.getElementById(id);
    nodes[++ptr] = addPtr(node.parentNode);
    nodes[++ptr] = addPtr(node.nextSibling);
    return (nodes[++ptr] = addPtr(node)) && ptr;
  },
  'createElement'(tagName) {
    const node = recycler.create(tagName);
    return node.asmDomPtr || (nodes[++ptr] = addPtr(node)) && ptr;
  },
  'createElementNS'(namespaceURI, qualifiedName) {
    const node = recycler.create(namespaceURI, qualifiedName);
    return node.asmDomPtr || (nodes[++ptr] = addPtr(node, namespaceURI)) && ptr;
  },
  'createTextNode'(text) {
    const node = recycler.createText(text);
    return node.asmDomPtr || (nodes[++ptr] = addPtr(node)) && ptr;
  },
  'createComment'(text) {
    const node = recycler.createComment(text);
    return node.asmDomPtr || (nodes[++ptr] = addPtr(node)) && ptr;
  },
  'insertBefore'(parentNodePtr, newNodePtr, referenceNodePtr) {
    nodes[parentNodePtr].insertBefore(nodes[newNodePtr], nodes[referenceNodePtr]);
  },
  'removeChild'(childPtr) {
    recycler.collect(nodes[childPtr]);
  },
  'appendChild'(parentPtr, childPtr) {
    nodes[parentPtr].appendChild(nodes[childPtr]);
  },
  'removeAttribute'(nodePtr, attr) {
    nodes[nodePtr].removeAttribute(attr);
  },
  'setAttribute'(nodePtr, attr, value) {
    nodes[nodePtr].setAttribute(attr, value);
  },
  'setProp'(nodePtr, key, value) {
    nodes[nodePtr][key] = value;
  },
  'parentNode': nodePtr => nodes[nodePtr].parentNode.asmDomPtr,
  'nextSibling': nodePtr => (
    nodes[nodePtr] && nodes[nodePtr].nextSibling && nodes[nodePtr].nextSibling.asmDomPtr ||
    0
  ),
  'setTextContent': (nodePtr, text) => {
    nodes[nodePtr].textContent = text;
  },
  'commit': (vnodePtr) => {
    requestAnimationFrame(() => {
      const op = window.asmDomHelpers.vnodesData[vnodePtr].operations;
      for (let i = 0, n = op.length; i < n; i++) {
        domApi[op[i][0]](op[i][1], op[i][2], op[i][3]);
      }
      if (window.asmDomHelpers.vnodesData[vnodePtr].callback) {
        setTimeout(window.asmDomHelpers.vnodesData[vnodePtr].callback);
      }
    });
  },
};

export default domApi;

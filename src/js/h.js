const getChildren = (lib, arr) => {
  const result = new lib.VNodeVector();
  arr.forEach(x => {
    if (typeof x === 'string') {
      result.push_back(new lib.VNode(x));
    } else if (x) {
      result.push_back(x);
    }
  });
  return result;
};

const objToProps = (lib, obj) => {
  const result = new lib.MapStringString();
  Object.keys(obj).forEach(x => {
    if (typeof obj[x] === 'string') {
      result.set(x, obj[x]);
    } else if (obj[x]) {
      result.set(x, String(obj[x]));
    }
  });
  return result;
};

export const getHFunction = (lib) => (a, b, c, d) => {
  let sel = '';
  let text = '';
  let props;
  let children;
  if (b === undefined) {
    sel = a;
  } else if (c === undefined) {
    if (Array.isArray(b)) {
      sel = a;
      children = getChildren(lib, b);
      props = new lib.MapStringString();
    } else if (b instanceof lib.VNode) {
      sel = a;
      children = getChildren(lib, [b]);
      props = new lib.MapStringString();
    } else {
      switch (typeof b) {
        case 'boolean':
          text = a;
          break;
        case 'string':
          sel = a;
          text = b;
          break;
        case 'object':
          sel = a;
          props = objToProps(lib, b);
          break;
        default:
          throw new Error('Invalid argument: ', b);
      }
    }
  } else if (d === undefined) {
    if (Array.isArray(c)) {
      sel = a;
      props = objToProps(lib, b);
      children = getChildren(lib, c);
    } else if (c instanceof lib.VNode) {
      sel = a;
      props = objToProps(lib, b);
      children = getChildren(lib, [c]);
    } else {
      switch (typeof c) {
        case 'string':
          sel = a;
          props = objToProps(lib, b);
          text = c;
          break;
        default:
          throw new Error('Invalid argument: ', c);
      }
    }
  }
  let result;
  if (props && children) {
    result = new lib.VNode(sel, props.get('key') || '', text, props, children);
    props.delete();
    children.delete();
  } else if (props) {
    result = new lib.VNode(sel, '', text, props);
    props.delete();
  } else {
    result = new lib.VNode(sel, text);
  }
  return result;
};

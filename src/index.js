import h from './js/h';
import patch from './js/patch';
import diff from './js/diff';
import domApi from './js/domApi';

// import() is compiled to require.ensure, this is a polyfill for nodejs
// an alternative solution is needed
if (typeof require.ensure !== 'function') require.ensure = (d, c) => { c(require); };

export default (config) => {
  config = config || {};
  let result;
  if ((config.useWasm || 'WebAssembly' in window) && !config.useAsmJS) {
    result = import('../compiled/wasm/asm-dom.wasm')
                  .then((wasm) => {
                    config.wasmBinary = new Uint8Array(wasm);
                  })
                  .then(() => import('../compiled/wasm/asm-dom.js'));
  } else {
    result = import('../compiled/asmjs/asm-dom.asm.js');
  }

  return result
    .then(lib => lib(config))
    .then((lib) => {
      if (!window && global) global.window = {};

      window.asmDom = lib;

      lib.h = h;
      lib.patch = patch;
      lib.deleteVNode = (oldVnode) => {
        window.asmDomHelpers.vnodesData[oldVnode] = undefined;
        lib._deleteVNode(oldVnode);
      };

      window.asmDomHelpers = {
        domApi,
        vnodesData: {},
        diff,
      };

      return lib;
    });
};

import { jsdom } from 'jsdom';

global.document = jsdom('<!doctype html><html><body></body></html>');
global.window = document.defaultView;
global.navigator = global.window.navigator;
global.HTMLElement = global.window.HTMLElement;
if (!global.window.requestAnimationFrame) {
  global.window.requestAnimationFrame = callback => callback();
}

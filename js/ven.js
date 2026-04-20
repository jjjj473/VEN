(function (global, factory) {
  if (typeof module === 'object' && typeof module.exports === 'object') {
    module.exports = factory();
  } else {
    global.VEN = factory();
  }
})(typeof window !== 'undefined' ? window : globalThis, function () {
  'use strict';

  const plugins = new Map();

  function assertString(value, name) {
    if (typeof value !== 'string') {
      throw new TypeError(name + ' must be a string');
    }
  }

  function countLines(text) {
    assertString(text, 'text');
    if (text.length === 0) {
      return 0;
    }

    let lines = 0;
    for (let i = 0; i < text.length; i += 1) {
      if (text[i] === '\n') {
        lines += 1;
      }
    }

    return lines;
  }

  function countWords(text) {
    assertString(text, 'text');
    const trimmed = text.trim();
    if (trimmed.length === 0) {
      return 0;
    }
    return trimmed.split(/\s+/).length;
  }

  function countBytes(text) {
    assertString(text, 'text');
    if (typeof TextEncoder !== 'undefined') {
      return new TextEncoder().encode(text).length;
    }
    if (typeof Buffer !== 'undefined') {
      return Buffer.byteLength(text, 'utf8');
    }

    // Conservative fallback for older runtimes.
    return unescape(encodeURIComponent(text)).length;
  }

  function use(name, fn) {
    assertString(name, 'name');
    if (typeof fn !== 'function') {
      throw new TypeError('plugin must be a function');
    }
    plugins.set(name, fn);
    return api;
  }

  function listPlugins() {
    return Array.from(plugins.keys());
  }

  function runPlugin(name, payload) {
    assertString(name, 'name');
    if (!plugins.has(name)) {
      throw new Error('Unknown plugin: ' + name);
    }
    return plugins.get(name)(payload);
  }

  function scan(text, options) {
    const config = options || {};
    const metrics = {
      lines: countLines(text),
      words: countWords(text),
      bytes: countBytes(text)
    };

    if (config.plugins === 'none' || config.plugins == null) {
      return { metrics: metrics, pluginResults: {} };
    }

    const requested = config.plugins === 'all' ? listPlugins() : config.plugins;
    const pluginList = Array.isArray(requested) ? requested : [requested];
    const pluginResults = {};

    for (const name of pluginList) {
      pluginResults[name] = runPlugin(name, { text: text, metrics: metrics });
    }

    return { metrics: metrics, pluginResults: pluginResults };
  }

  const api = {
    version: '0.1.0',
    countLines,
    countWords,
    countBytes,
    scan,
    use,
    listPlugins,
    runPlugin
  };

  return api;
});

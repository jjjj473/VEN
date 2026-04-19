#!/usr/bin/env node

(function (root, factory) {
  if (typeof module === 'object' && module.exports) {
    module.exports = factory();
  } else {
    root.Markup = factory();
  }
})(typeof globalThis !== 'undefined' ? globalThis : this, function () {
  function classifyLine(line, inCodeBlockState) {
    if (line.trimStart().startsWith('```')) {
      return { kind: 'fence', inCodeBlock: !inCodeBlockState };
    }
    if (inCodeBlockState) {
      return { kind: 'code_block', text: line };
    }
    if (!line.trim()) {
      return { kind: 'empty' };
    }

    const heading = line.match(/^(#{1,6})\s+(.*)$/);
    if (heading) return { kind: 'heading', level: heading[1].length, text: heading[2] };

    const list = line.match(/^[-*]\s+(.*)$/);
    if (list) return { kind: 'list_item', text: list[1] };

    const quote = line.match(/^>\s?(.*)$/);
    if (quote) return { kind: 'blockquote', text: quote[1] };

    return { kind: 'paragraph', text: line };
  }

  function scoreText(text) {
    const trimmed = text.trim();
    const punctuation = (trimmed.match(/[.,;:!?()[\]{}'"`~@#$%^&*+=|\\/-]/g) || []).length;
    const complexity = trimmed.length > 0 ? trimmed.length / 22 + punctuation / 5 : 0;
    const smoothness = trimmed.length > 0 ? 1 / (1 + complexity / 3.2) : 0;
    return { complexity, smoothness };
  }

  function parseMarkup(input) {
    const lines = String(input).split(/\r?\n/);
    const components = [];
    let inCodeBlock = false;

    lines.forEach((rawLine, i) => {
      const token = classifyLine(rawLine, inCodeBlock);
      if (token.kind === 'fence') {
        inCodeBlock = token.inCodeBlock;
        return;
      }
      if (token.kind === 'empty') {
        return;
      }

      const text = (token.text ?? '').trim();
      const { complexity, smoothness } = scoreText(text);
      components.push({
        type: token.kind,
        level: token.level ?? 0,
        text,
        line: i + 1,
        complexity,
        smoothness,
      });
    });

    return { components };
  }

  function escapeHtml(text) {
    return text
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }

  function renderMarkupHtml(document) {
    return document.components
      .map((component) => {
        const text = escapeHtml(component.text);
        switch (component.type) {
          case 'heading':
            return `<h${component.level} data-smooth="${component.smoothness.toFixed(2)}">${text}</h${component.level}>`;
          case 'list_item':
            return `<li data-complexity="${component.complexity.toFixed(2)}">${text}</li>`;
          case 'blockquote':
            return `<blockquote>${text}</blockquote>`;
          case 'code_block':
            return `<pre><code>${text}</code></pre>`;
          case 'paragraph':
          default:
            return `<p>${text}</p>`;
        }
      })
      .join('\n');
  }

  function diagnostics(document) {
    return document.components
      .map(
        (component) =>
          `  - line ${String(component.line).padStart(3, ' ')}: ${component.type.padEnd(10, ' ')} smooth=${component.smoothness.toFixed(2)} complexity=${component.complexity.toFixed(2)}`
      )
      .join('\n');
  }

  return {
    parseMarkup,
    renderMarkupHtml,
    diagnostics,
  };
});

if (typeof require === 'function' && typeof module === 'object' && require.main === module) {
  const fs = require('fs');
  const Markup = module.exports;
  const file = process.argv[2];

  if (!file) {
    console.error('Usage: node scripts/markup.js <file>');
    process.exit(1);
  }

  try {
    const content = fs.readFileSync(file, 'utf8');
    const document = Markup.parseMarkup(content);
    console.log(`[markup-js] Parsed ${document.components.length} components from ${file}`);
    if (document.components.length > 0) {
      console.log(Markup.diagnostics(document));
    }
    console.log('[markup-js] Render preview:');
    console.log(Markup.renderMarkupHtml(document));
  } catch (error) {
    console.error(`[markup-js] Unable to read ${file}: ${error.message}`);
    process.exit(1);
  }
}

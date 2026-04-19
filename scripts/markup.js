#!/usr/bin/env node

/**
 * markup: a richer markdown successor implemented in JavaScript.
 *
 * Exports reusable parsing and rendering functions and also provides
 * a CLI that prints component diagnostics + HTML preview.
 */

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
  if (heading) {
    return { kind: 'heading', level: heading[1].length, text: heading[2] };
  }

  const list = line.match(/^[-*]\s+(.*)$/);
  if (list) {
    return { kind: 'list_item', text: list[1] };
  }

  const quote = line.match(/^>\s?(.*)$/);
  if (quote) {
    return { kind: 'blockquote', text: quote[1] };
  }

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
  const lines = input.split(/\r?\n/);
  const components = [];
  let inCodeBlock = false;

  lines.forEach((rawLine, i) => {
    const lineNo = i + 1;
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
      line: lineNo,
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
    .map((c) => {
      const t = escapeHtml(c.text);
      switch (c.type) {
        case 'heading':
          return `<h${c.level} data-smooth="${c.smoothness.toFixed(2)}">${t}</h${c.level}>`;
        case 'list_item':
          return `<li data-complexity="${c.complexity.toFixed(2)}">${t}</li>`;
        case 'blockquote':
          return `<blockquote>${t}</blockquote>`;
        case 'code_block':
          return `<pre><code>${t}</code></pre>`;
        case 'paragraph':
        default:
          return `<p>${t}</p>`;
      }
    })
    .join('\n');
}

function diagnostics(document) {
  return document.components
    .map(
      (c) =>
        `  - line ${String(c.line).padStart(3, ' ')}: ${c.type.padEnd(10, ' ')} smooth=${c.smoothness.toFixed(2)} complexity=${c.complexity.toFixed(2)}`
    )
    .join('\n');
}

module.exports = {
  parseMarkup,
  renderMarkupHtml,
  diagnostics,
};

if (require.main === module) {
  const fs = require('fs');
  const file = process.argv[2];

  if (!file) {
    console.error('Usage: node scripts/markup.js <file>');
    process.exit(1);
  }

  let content;
  try {
    content = fs.readFileSync(file, 'utf8');
  } catch (err) {
    console.error(`[markup-js] Unable to read ${file}: ${err.message}`);
    process.exit(1);
  }

  const doc = parseMarkup(content);
  console.log(`[markup-js] Parsed ${doc.components.length} components from ${file}`);
  if (doc.components.length > 0) {
    console.log(diagnostics(doc));
  }
  console.log('[markup-js] Render preview:');
  console.log(renderMarkupHtml(doc));
}

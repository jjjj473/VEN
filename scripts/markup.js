#!/usr/bin/env node

(function (root, factory) {
  if (typeof module === 'object' && module.exports) {
    module.exports = factory();
  } else {
    root.Markup = factory();
  }
})(typeof globalThis !== 'undefined' ? globalThis : this, function () {
  const SEVERITY = {
    ERROR: 'error',
    WARNING: 'warning',
    INFO: 'info',
  };

  function pushError(errors, line, code, message, suggestion, severity = SEVERITY.WARNING) {
    errors.push({ line, code, message, suggestion, severity });
  }

  function scoreText(text) {
    const trimmed = text.trim();
    const punctuation = (trimmed.match(/[.,;:!?()[\]{}'"`~@#$%^&*+=|\\/-]/g) || []).length;
    const complexity = trimmed.length > 0 ? trimmed.length / 22 + punctuation / 5 : 0;
    const smoothness = trimmed.length > 0 ? 1 / (1 + complexity / 3.2) : 0;
    return { complexity, smoothness };
  }

  function tokenizeInline(text, line, errors) {
    const tokens = [];
    let i = 0;

    while (i < text.length) {
      if (text.startsWith('**', i)) {
        const end = text.indexOf('**', i + 2);
        if (end === -1) {
          pushError(errors, line, 'INLINE_UNCLOSED_BOLD', 'Unclosed bold marker (**).', 'Add a closing **.', SEVERITY.ERROR);
          tokens.push({ type: 'text', value: text.slice(i) });
          break;
        }
        tokens.push({ type: 'bold', value: text.slice(i + 2, end) });
        i = end + 2;
        continue;
      }

      if (text.startsWith('*', i)) {
        const end = text.indexOf('*', i + 1);
        if (end === -1) {
          pushError(errors, line, 'INLINE_UNCLOSED_ITALIC', 'Unclosed italic marker (*).', 'Add a closing *.', SEVERITY.ERROR);
          tokens.push({ type: 'text', value: text.slice(i) });
          break;
        }
        tokens.push({ type: 'italic', value: text.slice(i + 1, end) });
        i = end + 1;
        continue;
      }

      if (text.startsWith('`', i)) {
        const end = text.indexOf('`', i + 1);
        if (end === -1) {
          pushError(errors, line, 'INLINE_UNCLOSED_CODE', 'Unclosed inline code marker (`).', 'Add a closing `.', SEVERITY.ERROR);
          tokens.push({ type: 'text', value: text.slice(i) });
          break;
        }
        tokens.push({ type: 'code', value: text.slice(i + 1, end) });
        i = end + 1;
        continue;
      }

      if (text.startsWith('[', i)) {
        const endText = text.indexOf(']', i + 1);
        const openUrl = text.indexOf('(', endText + 1);
        const endUrl = text.indexOf(')', openUrl + 1);

        if (endText > -1 && openUrl === endText + 1 && endUrl > -1) {
          tokens.push({
            type: 'link',
            label: text.slice(i + 1, endText),
            href: text.slice(openUrl + 1, endUrl),
          });
          i = endUrl + 1;
          continue;
        }

        pushError(
          errors,
          line,
          'INLINE_BAD_LINK',
          'Malformed link syntax. Expected [label](url).',
          'Use full link format, for example: [Docs](https://example.com).',
          SEVERITY.ERROR
        );
      }

      let next = i + 1;
      while (next < text.length && !['*', '`', '['].includes(text[next])) {
        next++;
      }
      tokens.push({ type: 'text', value: text.slice(i, next) });
      i = next;
    }

    return tokens;
  }

  function classifyBlock(line, inCodeBlockState) {
    if (line.trimStart().startsWith('```')) {
      const lang = line.trim().slice(3).trim();
      return { kind: 'fence', inCodeBlock: !inCodeBlockState, language: lang || null };
    }

    if (inCodeBlockState) {
      return { kind: 'code_block_line', text: line };
    }

    if (!line.trim()) {
      return { kind: 'empty' };
    }

    const heading = line.match(/^(#{1,6})\s+(.*)$/);
    if (heading) return { kind: 'heading', level: heading[1].length, text: heading[2] };

    const taskItem = line.match(/^[-*]\s+\[( |x|X)\]\s+(.*)$/);
    if (taskItem) return { kind: 'task_item', checked: taskItem[1].toLowerCase() === 'x', text: taskItem[2] };

    const list = line.match(/^[-*]\s+(.*)$/);
    if (list) return { kind: 'list_item', text: list[1] };

    const quote = line.match(/^>\s?(.*)$/);
    if (quote) return { kind: 'blockquote', text: quote[1] };

    if (line.includes('|') && /^\|?.+\|.+\|?$/.test(line.trim())) {
      return { kind: 'table_row', text: line };
    }

    return { kind: 'paragraph', text: line };
  }

  function parseMarkup(input, options = {}) {
    const strict = Boolean(options.strict);
    const lines = String(input).split(/\r?\n/);
    const components = [];
    const errors = [];
    let inCodeBlock = false;
    let currentCodeBlock = null;
    let lastHeadingLevel = 0;

    lines.forEach((rawLine, i) => {
      const lineNumber = i + 1;
      const token = classifyBlock(rawLine, inCodeBlock);

      if (rawLine.length > 140) {
        pushError(errors, lineNumber, 'LINE_TOO_LONG', 'Line exceeds 140 characters.', 'Wrap long lines to improve readability.', SEVERITY.INFO);
      }

      if (rawLine.includes('\t')) {
        pushError(errors, lineNumber, 'TAB_CHARACTER', 'Tab character found.', 'Replace tabs with spaces for consistent formatting.');
      }

      if (token.kind === 'fence') {
        inCodeBlock = token.inCodeBlock;
        if (inCodeBlock) {
          currentCodeBlock = {
            type: 'code_block',
            language: token.language,
            lineStart: lineNumber,
            lines: [],
          };
        } else if (currentCodeBlock) {
          const text = currentCodeBlock.lines.join('\n');
          const score = scoreText(text);
          components.push({
            type: 'code_block',
            level: 0,
            text,
            line: currentCodeBlock.lineStart,
            language: currentCodeBlock.language,
            complexity: score.complexity,
            smoothness: score.smoothness,
            tokens: [{ type: 'code', value: text }],
          });
          currentCodeBlock = null;
        }
        return;
      }

      if (inCodeBlock) {
        currentCodeBlock.lines.push(rawLine);
        return;
      }

      if (token.kind === 'empty') {
        return;
      }

      if (token.kind === 'heading' && token.level > lastHeadingLevel + 1 && lastHeadingLevel !== 0) {
        pushError(
          errors,
          lineNumber,
          'HEADING_JUMP',
          `Heading level jumped from H${lastHeadingLevel} to H${token.level}.`,
          `Use H${lastHeadingLevel + 1} before H${token.level} for better structure.`
        );
      }

      if (token.kind === 'heading') {
        lastHeadingLevel = token.level;
      }

      if (token.kind === 'table_row') {
        const pipeCount = (rawLine.match(/\|/g) || []).length;
        if (pipeCount < 2) {
          pushError(errors, lineNumber, 'TABLE_PIPE_COUNT', 'Table row has insufficient pipe separators.', 'Use at least two pipes, for example: | a | b |.', SEVERITY.ERROR);
        }
      }

      const text = (token.text ?? '').trim();
      const score = scoreText(text);
      const inlineTokens = tokenizeInline(text, lineNumber, errors);

      components.push({
        type: token.kind,
        level: token.level ?? 0,
        checked: token.checked,
        text,
        line: lineNumber,
        complexity: score.complexity,
        smoothness: score.smoothness,
        tokens: inlineTokens,
      });
    });

    if (inCodeBlock) {
      pushError(errors, lines.length, 'CODE_FENCE_UNCLOSED', 'Code block fence was not closed.', 'Add a closing ``` fence.', SEVERITY.ERROR);
    }

    if (strict && errors.some((error) => error.severity === SEVERITY.ERROR)) {
      const message = formatErrors(errors.filter((error) => error.severity === SEVERITY.ERROR));
      const parseError = new Error(`Strict mode failed with markup errors:\n${message}`);
      parseError.name = 'MarkupParseError';
      parseError.errors = errors;
      throw parseError;
    }

    return {
      components,
      errors,
      meta: {
        lineCount: lines.length,
        componentCount: components.length,
        errorCount: errors.length,
        hasFatal: errors.some((error) => error.severity === SEVERITY.ERROR),
      },
    };
  }

  function escapeHtml(text) {
    return String(text)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }

  function renderInline(tokens) {
    return tokens
      .map((token) => {
        switch (token.type) {
          case 'bold':
            return `<strong>${escapeHtml(token.value)}</strong>`;
          case 'italic':
            return `<em>${escapeHtml(token.value)}</em>`;
          case 'code':
            return `<code>${escapeHtml(token.value)}</code>`;
          case 'link':
            return `<a href="${escapeHtml(token.href)}">${escapeHtml(token.label)}</a>`;
          case 'text':
          default:
            return escapeHtml(token.value);
        }
      })
      .join('');
  }

  function renderMarkupHtml(document) {
    return document.components
      .map((component) => {
        const inline = renderInline(component.tokens || [{ type: 'text', value: component.text }]);
        switch (component.type) {
          case 'heading':
            return `<h${component.level} data-smooth="${component.smoothness.toFixed(2)}">${inline}</h${component.level}>`;
          case 'task_item':
            return `<li data-task="true"><input type="checkbox" ${component.checked ? 'checked' : ''} disabled /> ${inline}</li>`;
          case 'list_item':
            return `<li data-complexity="${component.complexity.toFixed(2)}">${inline}</li>`;
          case 'blockquote':
            return `<blockquote>${inline}</blockquote>`;
          case 'table_row':
            return `<div data-table-row="true">${inline}</div>`;
          case 'code_block':
            return `<pre><code data-language="${escapeHtml(component.language || '')}">${escapeHtml(component.text)}</code></pre>`;
          case 'paragraph':
          default:
            return `<p>${inline}</p>`;
        }
      })
      .join('\n');
  }

  function diagnostics(document) {
    const componentRows = document.components.map(
      (component) =>
        `  - line ${String(component.line).padStart(3, ' ')}: ${component.type.padEnd(12, ' ')} smooth=${component.smoothness.toFixed(2)} complexity=${component.complexity.toFixed(2)}`
    );
    const errorSummary = document.errors.length
      ? [`\n[errors] ${document.errors.length} issue(s) detected.`]
      : ['\n[errors] No issues detected.'];
    return componentRows.concat(errorSummary).join('\n');
  }

  function formatErrors(errors) {
    if (!errors || errors.length === 0) {
      return 'No markup errors found.';
    }
    return errors
      .map(
        (error) =>
          `[${error.severity.toUpperCase()}] line ${error.line} ${error.code}: ${error.message} Fix: ${error.suggestion}`
      )
      .join('\n');
  }

  function suggestFixes(input, options = {}) {
    const document = parseMarkup(input, options);
    return formatErrors(document.errors);
  }


  function defaultAiRules() {
    return [
      'Preserve author intent and document meaning.',
      'Fix syntax errors before style changes.',
      'Never invent links, references, or code semantics.',
      'Return concrete, minimal edits with explanations.',
      'Flag uncertain changes instead of guessing.',
    ];
  }

  function buildAiPrompt(input, document, options = {}) {
    const provider = options.provider || 'openai';
    const model = options.model || 'gpt-4.1-mini';
    const rules = options.rules && options.rules.length ? options.rules : defaultAiRules();
    const errorLog = formatErrors(document.errors);

    return [
      `You are a markup repair assistant for provider=${provider}, model=${model}.`,
      'Follow these AI rules strictly:',
      ...rules.map((rule, idx) => `${idx + 1}. ${rule}`),
      '',
      'Detected issues:',
      errorLog,
      '',
      'Original document:',
      input,
      '',
      'Task:',
      '- Fix markup issues.',
      '- Keep content as close as possible to the original.',
      '- Return JSON with keys: summary, fixed_markup, rationale.',
    ].join('\n');
  }

  function buildProviderRequest(provider, model, prompt, options = {}) {
    const maxTokens = options.maxTokens || 1200;
    if (provider === 'anthropic') {
      return {
        provider,
        endpoint: options.endpoint || 'https://api.anthropic.com/v1/messages',
        headers: {
          'content-type': 'application/json',
          'x-api-key': options.apiKey || '<ANTHROPIC_API_KEY>',
          'anthropic-version': '2023-06-01',
        },
        body: {
          model: model || 'claude-3-7-sonnet-latest',
          max_tokens: maxTokens,
          messages: [{ role: 'user', content: prompt }],
        },
      };
    }

    return {
      provider: 'openai',
      endpoint: options.endpoint || 'https://api.openai.com/v1/chat/completions',
      headers: {
        'content-type': 'application/json',
        authorization: `Bearer ${options.apiKey || '<OPENAI_API_KEY>'}`,
      },
      body: {
        model: model || 'gpt-4.1-mini',
        temperature: options.temperature ?? 0.1,
        response_format: { type: 'json_object' },
        messages: [
          { role: 'system', content: 'You fix markup documents while preserving intent.' },
          { role: 'user', content: prompt },
        ],
      },
    };
  }

  function extractAiText(provider, responsePayload) {
    try {
      if (provider === 'anthropic') {
        return responsePayload.content?.[0]?.text || '';
      }
      return responsePayload.choices?.[0]?.message?.content || '';
    } catch (err) {
      return '';
    }
  }

  async function aiAssist(input, options = {}) {
    const provider = options.provider || 'openai';
    const model = options.model;
    const document = parseMarkup(input, { strict: false });
    const prompt = buildAiPrompt(input, document, options);
    const request = buildProviderRequest(provider, model, prompt, options);

    if (typeof options.transport !== 'function') {
      return {
        mode: 'preview',
        prompt,
        request,
        errors: document.errors,
        guidance: 'Pass a transport(request) function to execute this against an AI provider.',
      };
    }

    const responsePayload = await options.transport(request);
    const rawText = extractAiText(provider, responsePayload);

    let parsed;
    try {
      parsed = JSON.parse(rawText);
    } catch (err) {
      parsed = {
        summary: 'AI response was not valid JSON; returning raw output.',
        fixed_markup: input,
        rationale: rawText,
      };
    }

    return {
      mode: 'executed',
      prompt,
      request,
      response: responsePayload,
      parsed,
    };
  }

  return {
    parseMarkup,
    renderMarkupHtml,
    diagnostics,
    formatErrors,
    suggestFixes,
    buildAiPrompt,
    buildProviderRequest,
    aiAssist,
    defaultAiRules,
    severity: SEVERITY,
  };
});

if (typeof require === 'function' && typeof module === 'object' && require.main === module) {
  const fs = require('fs');
  const Markup = module.exports;
  const file = process.argv[2];
  const strict = process.argv.includes('--strict');
  const aiPreview = process.argv.includes('--ai-preview');
  const providerArg = process.argv.find((arg) => arg.startsWith('--provider='));
  const provider = providerArg ? providerArg.split('=')[1] : 'openai';

  if (!file) {
    console.error('Usage: node scripts/markup.js <file> [--strict]');
    process.exit(1);
  }

  try {
    const content = fs.readFileSync(file, 'utf8');
    const document = Markup.parseMarkup(content, { strict });
    console.log(`[markup-js] Parsed ${document.components.length} components from ${file}`);
    console.log(Markup.diagnostics(document));
    console.log('[markup-js] Error log:');
    console.log(Markup.formatErrors(document.errors));
    console.log('[markup-js] Render preview:');
    console.log(Markup.renderMarkupHtml(document));

    if (aiPreview) {
      const preview = Markup.aiAssist(content, { provider });
      Promise.resolve(preview).then((result) => {
        console.log('[markup-js] AI Assist Preview:');
        console.log(result.guidance || '');
        console.log(JSON.stringify(result.request, null, 2));
      });
    }
  } catch (error) {
    console.error(`[markup-js] ${error.message}`);
    if (error.errors) {
      console.error(Markup.formatErrors(error.errors));
    }
    process.exit(1);
  }
}

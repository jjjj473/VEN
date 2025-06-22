import re
import io
import sys
import os
import html


def render(file_path, context=None, _is_root=True):
    """Render a .ven file and return the resulting string."""
    if context is None:
        context = {}

    # state for advanced templating features
    context.setdefault('_layout', None)
    context.setdefault('_blocks', {})
    capture_stack = context.setdefault('_capture_stack', [])
    output_stack = context.setdefault('_output_stack', [])

    # helper functions available within VEN scripts
    def include(path):
        target = os.path.join(os.path.dirname(file_path), path)
        return render(target, context, _is_root=False)

    def echo(*args):
        print(*args, sep='', end='')

    def layout(path):
        context['_layout'] = os.path.join(os.path.dirname(file_path), path)

    def start_block(name):
        buf = []
        capture_stack.append((name, buf))
        output_stack.append(buf)

    def end_block():
        name, buf = capture_stack.pop()
        output_stack.pop()
        context['_blocks'][name] = ''.join(buf)

    def yield_block(name):
        return context['_blocks'].get(name, '')

    context.setdefault('include', include)
    context.setdefault('echo', echo)
    context.setdefault('escape', html.escape)
    context.setdefault('layout', layout)
    context.setdefault('start_block', start_block)
    context.setdefault('end_block', end_block)
    context.setdefault('yield_block', yield_block)
    with open(file_path, 'r', encoding='utf-8') as f:
        text = f.read()

    output = []
    output_stack.append(output)
    pos = 0
    pattern = re.compile(r"<\?VEN(=)?(.*?)\?>", re.DOTALL)
    for match in pattern.finditer(text):
        output_stack[-1].append(text[pos:match.start()])
        is_expr = match.group(1)
        code = match.group(2).strip()
        if is_expr:
            try:
                result = eval(code, context)
                output_stack[-1].append(str(result))
            except Exception as e:
                output_stack[-1].append(f"[Error: {e}]")
        else:
            old_stdout = sys.stdout
            sys.stdout = io.StringIO()
            try:
                exec(code, context)
                output_stack[-1].append(sys.stdout.getvalue())
            except Exception as e:
                output_stack[-1].append(f"[Error: {e}]")
            finally:
                sys.stdout = old_stdout
        pos = match.end()
    output_stack[-1].append(text[pos:])
    output_stack.pop()
    result = ''.join(output)

    # apply layout if this is the top-level render call
    if _is_root and context.get('_layout'):
        layout_path = context.pop('_layout')
        context['_blocks'].setdefault('content', result)
        return render(layout_path, context, _is_root=True)

    return result


def main():
    import argparse
    parser = argparse.ArgumentParser(description='VEN language interpreter')
    parser.add_argument('source', help='Path to the .ven file')
    args = parser.parse_args()

    # Provide common modules in the context
    ctx = {
        'time': __import__('time'),
        'os': os,
        'html': html,
    }
    result = render(args.source, ctx)
    print(result)


if __name__ == '__main__':
    main()

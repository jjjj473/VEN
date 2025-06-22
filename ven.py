import re
import io
import sys
import os
import html


def render(file_path, context=None):
    """Render a .ven file and return the resulting string."""
    if context is None:
        context = {}
    # helper functions available within VEN scripts
    def include(path):
        target = os.path.join(os.path.dirname(file_path), path)
        return render(target, context)

    def echo(*args):
        print(*args, sep='', end='')

    context.setdefault('include', include)
    context.setdefault('echo', echo)
    context.setdefault('escape', html.escape)
    with open(file_path, 'r', encoding='utf-8') as f:
        text = f.read()

    output = []
    pos = 0
    pattern = re.compile(r"<\?VEN(=)?(.*?)\?>", re.DOTALL)
    for match in pattern.finditer(text):
        output.append(text[pos:match.start()])
        is_expr = match.group(1)
        code = match.group(2).strip()
        if is_expr:
            try:
                result = eval(code, context)
                output.append(str(result))
            except Exception as e:
                output.append(f"[Error: {e}]")
        else:
            old_stdout = sys.stdout
            sys.stdout = io.StringIO()
            try:
                exec(code, context)
                output.append(sys.stdout.getvalue())
            except Exception as e:
                output.append(f"[Error: {e}]")
            finally:
                sys.stdout = old_stdout
        pos = match.end()
    output.append(text[pos:])
    return ''.join(output)


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

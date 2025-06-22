import re
import io
import sys
import os
import html
from http.server import SimpleHTTPRequestHandler, HTTPServer
import urllib.parse


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


def serve(directory='.', host='localhost', port=8000):
    """Start a simple HTTP server that renders .ven files on the fly."""
    os.chdir(directory)

    class VenHandler(SimpleHTTPRequestHandler):
        def do_GET(self):
            path, _, query = self.path.partition('?')
            fs_path = path.lstrip('/') or 'index.ven'
            if fs_path.endswith('.ven') and os.path.exists(fs_path):
                try:
                    query_dict = urllib.parse.parse_qs(query)
                    context = {'query': {k: v[0] if len(v) == 1 else v for k, v in query_dict.items()}}
                    result = render(fs_path, context)
                    self.send_response(200)
                    self.send_header('Content-type', 'text/html; charset=utf-8')
                    self.end_headers()
                    self.wfile.write(result.encode('utf-8'))
                except Exception as e:
                    self.send_response(500)
                    self.end_headers()
                    self.wfile.write(f'Error: {e}'.encode('utf-8'))
            else:
                super().do_GET()

    httpd = HTTPServer((host, port), VenHandler)
    print(f'Serving {directory} on http://{host}:{port} (Press CTRL+C to quit)')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass


def main():
    import argparse
    parser = argparse.ArgumentParser(description='VEN language interpreter')
    parser.add_argument('source', nargs='?', help='Path to the .ven file')
    parser.add_argument('--serve', action='store_true', help='Start an HTTP server')
    parser.add_argument('--host', default='localhost', help='Server host')
    parser.add_argument('--port', type=int, default=8000, help='Server port')
    args = parser.parse_args()

    # Provide common modules in the context
    ctx = {
        'time': __import__('time'),
        'os': os,
        'html': html,
    }

    if args.serve:
        serve('.', args.host, args.port)
        return

    if not args.source:
        parser.error('source is required unless --serve is used')

    result = render(args.source, ctx)
    print(result)


if __name__ == '__main__':
    main()

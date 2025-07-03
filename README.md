# VEN Text Editor Suite

This project demonstrates a minimal text editor that can be used from the
web or a desktop program. A small FastAPI backend stores the text so that
both interfaces stay in sync.

## Components

- **backend.py**: FastAPI server serving the web editor and a JSON API.
- **index.html**: Simple web-based text editor using `fetch` to load and
  save text.
- **client.c**: Tiny desktop program built with `libcurl` that retrieves
  or updates the text via the backend API.

## Setup

1. Install Python dependencies:

```bash
python3 -m pip install -r requirements.txt
```

2. Run the backend:

```bash
uvicorn backend:app --reload
```

3. Open `http://localhost:8000` in your browser to use the web editor.

4. Build the C client (requires `gcc` and `libcurl`):

```bash
make
```

5. Use the client to get or set text:

```bash
./client get
./client set "Hello world"
```

Both the desktop client and the web page share the same text data through
the Python backend.

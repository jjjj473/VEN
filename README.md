# VEN
VEN is a code editor built like Vim but have more to offer.

## Video demo server

This repository now includes a small FastAPI backend used by
`index.html` to upload and stream videos. To try it out:

```bash
pip install -r requirements.txt
python app.py
```

Then open `index.html` in your browser. The page will fetch the playlist
from `http://localhost:8000` and allow uploading new videos directly to
the server. You can also interact with the API using `client.py`, which
uses the `requests` package:

```bash
python client.py            # list uploaded videos
python client.py file1.mp4  # upload one or more files
```

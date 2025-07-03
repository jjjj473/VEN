import express from 'express';
import multer from 'multer';
import fs from 'fs';
import path from 'path';

const app = express();
const PORT = process.env.PORT || 8000;
const VIDEOS_DIR = path.join(__dirname, 'videos');
fs.mkdirSync(VIDEOS_DIR, { recursive: true });

const storage = multer.diskStorage({
  destination: (_req, _file, cb) => cb(null, VIDEOS_DIR),
  filename: (_req, file, cb) => cb(null, file.originalname)
});

const upload = multer({ storage });

app.get('/', (_req, res) => {
  res.type('html').send(`
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Cool Video Player</title>
  <style>
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      background: #0f0f0f;
      color: #fff;
      display: flex;
      height: 100vh;
      overflow: hidden;
    }
    #player-container {
      flex: 2;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      background: #181818;
      position: relative;
    }
    video {
      width: 90%;
      max-height: 80vh;
      border-radius: 8px;
      background: #000;
    }
    #controls {
      margin-top: 10px;
      display: flex;
      gap: 10px;
    }
    button {
      padding: 6px 12px;
      border: none;
      border-radius: 4px;
      background: #ff0000;
      color: #fff;
      cursor: pointer;
    }
    button:hover {
      background: #e50914;
    }
    #playlist {
      flex: 1;
      overflow-y: auto;
      background: #212121;
    }
    #playlist ul {
      list-style: none;
      padding: 0;
      margin: 0;
    }
    #playlist li {
      padding: 10px;
      cursor: pointer;
      border-bottom: 1px solid #333;
    }
    #playlist li.active {
      background: #383838;
    }
    #upload {
      display: none;
    }
  </style>
</head>
<body>
  <div id="player-container">
    <video id="video" controls></video>
    <div id="controls">
      <button id="skip">Skip</button>
      <button id="loop">Loop Off</button>
      <label for="upload" style="background:#ff0000;padding:6px 12px;border-radius:4px;cursor:pointer;">Upload</label>
      <input type="file" id="upload" accept="video/*" multiple>
    </div>
  </div>
  <div id="playlist">
    <ul id="list"></ul>
  </div>
<script>
const video = document.getElementById('video');
const list = document.getElementById('list');
const upload = document.getElementById('upload');
const skipBtn = document.getElementById('skip');
const loopBtn = document.getElementById('loop');

const playlist = [];
let current = 0;

async function loadPlaylist() {
  const res = await fetch('/videos');
  const data = await res.json();
  playlist.length = 0;
  data.videos.forEach(name => {
    playlist.push({ name, src: '/videos/' + encodeURIComponent(name) });
  });
  if (playlist.length) {
    current = 0;
    playCurrent();
  }
}

function renderList() {
  list.innerHTML = '';
  playlist.forEach((item, index) => {
    const li = document.createElement('li');
    li.textContent = item.name;
    li.className = index === current ? 'active' : '';
    li.addEventListener('click', () => {
      current = index;
      playCurrent();
    });
    list.appendChild(li);
  });
}

function playCurrent() {
  const item = playlist[current];
  video.src = item.src;
  renderList();
  video.play();
}

video.addEventListener('ended', () => {
  if (video.loop) return;
  current = (current + 1) % playlist.length;
  playCurrent();
});

skipBtn.addEventListener('click', () => {
  current = (current + 1) % playlist.length;
  playCurrent();
});

loopBtn.addEventListener('click', () => {
  video.loop = !video.loop;
  loopBtn.textContent = video.loop ? 'Loop On' : 'Loop Off';
});

upload.addEventListener('change', async (e) => {
  const files = Array.from(e.target.files);
  const form = new FormData();
  files.forEach(file => form.append('files', file));
  await fetch('/upload', { method: 'POST', body: form });
  await loadPlaylist();
});

loadPlaylist();
</script>
</body>
</html>`);
});

app.get('/videos', (_req, res) => {
  const files = fs.readdirSync(VIDEOS_DIR).filter(f => fs.statSync(path.join(VIDEOS_DIR, f)).isFile());
  res.json({ videos: files });
});

app.post('/upload', upload.array('files'), (req, res) => {
  const uploaded = (req.files as Express.Multer.File[]).map(f => f.originalname);
  res.json({ uploaded });
});

app.use('/videos', express.static(VIDEOS_DIR));

app.listen(PORT, () => {
  console.log(`Server listening on http://localhost:${PORT}`);
});

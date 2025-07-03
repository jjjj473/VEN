# VEN
VEN is a code editor built like Vim but has more to offer.

## TypeScript video demo server

The Python backend has been replaced with a minimal Node.js server
written in TypeScript. It serves an HTML page with a simple video
player, allowing uploads and playlist management.

### Running

```bash
npm install
npm run start
```

Open <http://localhost:8000> in your browser to use the player.
Uploaded files are stored in the `videos` directory.

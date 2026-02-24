const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const PORT = 8000;
const BACKEND_PATH = path.join(__dirname, 'backend', 'backend.exe');
const FRONTEND_PATH = path.join(__dirname, 'frontend');

let backend = null;
let requestQueue = [];
let isProcessing = false;

function startBackend() {
  if (backend) return;
  backend = spawn(BACKEND_PATH, [], { cwd: __dirname });
  backend.stderr.on('data', (chunk) => console.error('Backend:', chunk.toString()));
  backend.on('close', (code) => {
    backend = null;
    console.error('Backend exited with code', code);
  });
  console.log('Backend started (long-running, in-memory state)');
}

function processQueue() {
  if (isProcessing || requestQueue.length === 0 || !backend) return;
  isProcessing = true;
  const { body, callback } = requestQueue.shift();
  const msg = body.length + '\n' + body;

  let buffer = '';
  const onData = (chunk) => {
    buffer += chunk.toString();
    const idx = buffer.indexOf('\n');
    if (idx >= 0) {
      const len = parseInt(buffer.substring(0, idx), 10);
      const json = buffer.substring(idx + 1);
      if (json.length >= len) {
        backend.stdout.removeListener('data', onData);
        callback(json.substring(0, len));
        isProcessing = false;
        processQueue();
      }
    }
  };

  backend.stdout.on('data', onData);
  backend.stdin.write(msg);
}

function sendToBackend(body, callback) {
  requestQueue.push({ body, callback });
  if (backend) processQueue();
}

const server = http.createServer((req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') {
    res.writeHead(200);
    res.end();
    return;
  }

  if (req.method === 'POST') {
    let data = '';
    req.on('data', (chunk) => { data += chunk; });
    req.on('end', () => {
      startBackend();
      sendToBackend(data, (jsonBody) => {
        res.setHeader('Content-Type', 'application/json');
        res.writeHead(200);
        res.end(jsonBody);
      });
    });
    return;
  }

  let filePath = req.url === '/' ? '/index.html' : req.url;
  filePath = path.join(FRONTEND_PATH, filePath);
  fs.readFile(filePath, (err, data) => {
    if (err) {
      res.writeHead(404);
      res.end('File not found');
      return;
    }
    const ext = path.extname(filePath);
    const types = { '.html': 'text/html', '.css': 'text/css', '.js': 'application/javascript', '.json': 'application/json' };
    res.writeHead(200, { 'Content-Type': types[ext] || 'text/plain' });
    res.end(data);
  });
});

server.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
  startBackend();
});

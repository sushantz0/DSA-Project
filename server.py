#!/usr/bin/env python3
"""
Simple Python HTTP server that serves the frontend and routes JSON requests to the C++ backend.
"""

import http.server
import socketserver
import json
import os
import subprocess
from pathlib import Path

PORT = 8000
BACKEND_PATH = Path(__file__).parent / "backend" / "main.exe"
FRONTEND_PATH = Path(__file__).parent / "frontend"

class DisasterHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        """Serve static files from frontend directory"""
        # Change to frontend directory for serving files
        os.chdir(FRONTEND_PATH)
        
        # Default to index.html for root path
        if self.path == "/":
            self.path = "/index.html"
        
        super().do_GET()
    
    def do_POST(self):
        """Handle JSON requests and pipe to C++ backend"""
        try:
            content_length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_length).decode('utf-8')
            
            # Run C++ backend with the JSON input
            env = os.environ.copy()
            env['CONTENT_LENGTH'] = str(len(body))
            
            result = subprocess.run(
                [str(BACKEND_PATH)],
                input=body,
                capture_output=True,
                text=True,
                env=env
            )
            
            response = result.stdout
            
            # Send response
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(response.encode('utf-8'))
            
        except Exception as e:
            self.send_response(500)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            error = json.dumps({"error": str(e)})
            self.wfile.write(error.encode('utf-8'))

if __name__ == "__main__":
    os.chdir(FRONTEND_PATH)
    
    with socketserver.TCPServer(("", PORT), DisasterHandler) as httpd:
        print(f"Server running at http://localhost:{PORT}")
        print("Press Ctrl+C to stop")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped")

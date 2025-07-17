#!/usr/bin/env python3

import os
import cgi
import urllib.parse
import mimetypes

print()

FILES_DIR = "./files"
form = cgi.FieldStorage()
filename = form.getfirst("file", "")

# Sanitize and resolve path
safe_filename = os.path.basename(filename)
filepath = os.path.join(FILES_DIR, safe_filename)
fullpath = os.path.realpath(filepath)
allowed_dir = os.path.realpath(FILES_DIR)

# Start HTML
html = '''
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <link rel="stylesheet" href="/stile.css">
  <title>View File</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: #f8f8f8;
      color: #333;
      display: flex;
      justify-content: center;
      padding: 2rem;
    }
    .container {
      max-width: 800px;
      background-color: rgba(44, 62, 80, 0.5); /* semi-transparent background */
      padding: 2rem;
      border-radius: 8px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
      align-items: center;
      text-align: center;
      width: 100%;
    }

    .titolo {
      font-size: 30px;
    }

    pre {
      text-align: center;
      background: #f4f4f4;
      padding: 1rem;
      border-radius: 6px;
      overflow-x: auto;
      max-height: 500px;
    }
    img, embed, video, audio {
      max-width: 100%;
      margin-top: 1em;
    }
    .back {
      margin-top: 2rem;
    }
    .back a {
      color: #1d3557;
      text-decoration: none;
      font-weight: bold;
    }
    .back a:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="container">
'''

if not filename:
    html += "<h1 class='titolo'>❌ No file specified</h1>"
elif not fullpath.startswith(allowed_dir + os.sep):
    html += "<h1 class='titolo'>🚫 Access denied</h1>"
elif not os.path.exists(fullpath):
    html += f"<h1 class='titolo'>⚠️ File not found: {safe_filename}</h1>"
else:
    mime, _ = mimetypes.guess_type(fullpath)
    html += f"<h1 class='titolo'>📄 Viewing: {safe_filename}</h1>"

    if mime and mime.startswith("image"):
        html += f'<img src="/files/{urllib.parse.quote(safe_filename)}" alt="{safe_filename}"/>'
    elif mime == "application/pdf":
        html += f'<embed src="/files/{urllib.parse.quote(safe_filename)}" type="application/pdf" width="100%" height="600px" />'
    elif mime and mime.startswith("video"):
        html += f'<video controls><source src="/files/{urllib.parse.quote(safe_filename)}" type="{mime}">Your browser does not support video.</video>'
    elif mime and mime.startswith("audio"):
        html += f'<audio controls><source src="/files/{urllib.parse.quote(safe_filename)}" type="{mime}">Your browser does not support audio.</audio>'
    elif mime and mime.startswith("text"):
        try:
            with open(fullpath, 'r', encoding='utf-8') as f:
                content = f.read()
                html += f"<pre>{cgi.escape(content)}</pre>"
        except Exception as e:
            html += f"<p>❌ Could not read file: {e}</p>"
    else:
        html += "<p>📦 This file cannot be previewed directly in the browser.</p>"

html += '''
    <div class="back">
      <a href="/cgi-bin/serve_cgi_page.py">← Back to File List</a>
    </div>
  </div>
</body>
</html>
'''

print(html)

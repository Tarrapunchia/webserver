#!/usr/bin/env python3

import os
import urllib.parse

print()

FILES_DIR = "./files"
files = []
try:
    files = sorted(os.listdir(FILES_DIR))
except FileNotFoundError:
    os.makedirs(FILES_DIR)
    files = []

html = '''
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Files List</title>
  <link rel="stylesheet" href="/stile.css">
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
      width: 100%;
    }
    h1 {
      color: #e63946;
      text-align: center;
    }
    ul {
      list-style: none;
      padding: 0;
    }
    li {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 0.5rem 1rem;
      border-bottom: 1px solid #eee;
    }
    a {
      align-text: center;
    }
    a.file-link {
      color: #1d3557;
      text-decoration: none;
      flex-grow: 1;
    }
    a.file-link:hover {
      text-decoration: underline;
    }
    form {
      margin: 0;
    }
    .btn {
      background-color: transparent;
      border: none;
      color: #e63946;
      font-size: 1.2rem;
      cursor: pointer;
    }
    .back {
      display: block;
      margin-top: 2rem;
      text-align: center;
    }
    .back a {
      color: #1d3557;
      text-decoration: none;
      font-weight: bold;
    }
    .back a:hover {
      text-decoration: underline;
    }
    .add {
        align: center;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🗂️ File List</h1>
    <ul>
'''

for filename in files:
    FILES_DIR = "/files"

    html += f'''
      <li>
        <form method="POST" action="/cgi-bin/view_file.py" enctype="multipart/form-data">
          <input type="hidden" name="file" value="{filename}">
          <button class="btn" title="View">🔍</button>
        </form>
        <a href="{FILES_DIR + '/' + filename}">{filename}</a>
        <form method="POST" action="/cgi-bin/delete_file.py" enctype="multipart/form-data">
          <input type="hidden" name="deletefile" value="{filename}">
          <button class="btn" title="Delete">❌</button>
        </form>
      </li>
    '''

html += '''
    <li>
        <a class="add" href="/upload_file.html"><button>📤 Add a file</button></a>
    </ul>
    <br><br>
    <div class="back">
      <a href="/">← Back to Homepage</a>
    </div>
  </div>
  <a href="/cgi-bin/simple.sh" />
</body>
</html>
'''

print(html)

#!/usr/bin/env python3

import cgi
import os

print()  # Intestazioni e body separati da una riga vuota perche' si

# Secondo check su esistenza dir (il primo in execute_cgi)
delete_dir = "./files"
os.makedirs(delete_dir, exist_ok=True)

# Parsing del form
form = cgi.FieldStorage()

html_template = '''
<!DOCTYPE html>
<html lang="it">
<head>
<meta charset="UTF-8">
<title>Upload Result</title>
<link rel="stylesheet" href="/stile.css">

</head>
<body><br><br>
    <div class="container">
        <a href="/upload_file.html" />
        <p>
        {content} <br><br><br></p>
        <p>
        <button class="load">
            Back to Load File Page
        </button>
        <a href="/" />
        <button class="home">
            Back to Homepage
        </button>
        </p>
    </div>
</body>
</html>
'''

filename = form.getvalue("deletefile")
if filename:
    filename = os.path.basename(filename)
    filepath = os.path.join(delete_dir, filename)
    real_path = os.path.realpath(filepath)
    allowed_dir = os.path.realpath(delete_dir)

    if real_path.startswith(allowed_dir + os.sep):
        if os.path.exists(real_path):
            os.remove(real_path)
            content = f'''
            <h2>🗑️ File deleted successfully:</h2>
            <p><strong>{filename}</strong></p>
            '''
        else:
            content = f"<h2>⚠️ File not found: {filename}</h2>"
    else:
        content = "<h2>🚫 Security error: invalid path!</h2>"
else:
    content = "<h2>❌ Error: deletefile field missing or empty!</h2>"

print(html_template.format(content=content))
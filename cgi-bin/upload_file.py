#!/usr/bin/env python3

import cgi
import os

print()  # Intestazioni e body separati da una riga vuota perche' si

# Secondo check su esistenza dir (il primo in execute_cgi)
upload_dir = "./files"
os.makedirs(upload_dir, exist_ok=True)

# Parsing del form
form = cgi.FieldStorage()

cookie = os.environ.get("COOKIES", "")
header = ""

html_template = '''
<!DOCTYPE html>
<html lang="it">
<head>
<meta charset="UTF-8">
<title>Upload Result</title>
<link rel="stylesheet" href="/stile.css">
</head>
<body>
<br><br><br><br><br><br><br>
<div class="container">
<p style="align-items: center">
	{header}
	{content} <br><br><br>
		<button class="load"><a href="/upload_file.html">Back to Load File Page	</a></button>
		<button class="home"><a href="/">Back to Homepage</a></button>
</p>
</div>
</body>
</html>
'''

# Campo del file input
if "uploadfile" in form:
    fileitem = form["uploadfile"]
    if fileitem.filename:
        filename = os.path.basename(fileitem.filename)
        filepath = os.path.join(upload_dir, filename)
        print(filepath)
        with open(filepath, 'wb') as f:
            f.write(fileitem.file.read())

        content = f'''
        <h2>✅ Uploaded successfully the file:</h2>
        <p><strong>{filename}</strong></p>
        <img src="/files/{filename}" alt="{filename}">
        '''
    else:
        content = "<p>⚠️ Error: no file selected!</p>"
else:
    content = "<p>❌ Error: upload_file field missing!</p>"

print(html_template.format(content=content, header=header))
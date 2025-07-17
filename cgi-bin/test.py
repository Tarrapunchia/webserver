#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys
import requests
import json

def main():
    # # Enable CGI traceback for debugging
    cgitb.enable()
    
    # # Print required CGI headers first
    print("Content-Type: text/html")
    print()  # Empty line to separate headers from body
    
    # # Print HTML headers
    print("<html>")
    print("<head><title>CGI Test</title></head>")
    print("<body>")
    
    # # Process form data if any
    # form = cgi.FieldStorage()
    
    # # Print environment variables for debugging
    # print("<h2>Environment Variables:</h2>")
    # print("<pre>")
    # for key in sorted(os.environ.keys()):
    #     print(f"{key}: {os.environ[key]}")
    # print("</pre>")
    
    # # Check for form data
    # if form:
    #     print("<h2>Form Data:</h2>")
    #     for field in form.keys():
    #         print(f"<p>{field}: {form[field].value}</p>")
    # else:
    #     print("<p>No form data submitted</p>")
    
    # print("<h1>HELLO CGI</h1>")
    response = requests.get('https://dog.ceo/api/breeds/image/random')
    data = json.loads(response.text)
    img = data["message"]
    print(f'<img src="{img}" alt="Girl in a jacket" width="500" height="600")>')
    print(response.text)
    # print("<img")
    print("</body>")
    print("</html>")


if __name__ == "__main__":
    main()

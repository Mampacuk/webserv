#!/usr/bin/python3

import cgi, os

print("Content-Type: text/html\r")    # HTML is following
print("\r")                           # blank line, end of headers

form = cgi.FieldStorage()

# Get filename here
fileitem = form['file']
whereto = form['destination'].value

# Remove leading slash if present
if whereto.startswith('/'):
    whereto = whereto[1:]

if not whereto.endswith('/'):
   whereto += '/'

# Test if the file was uploaded
if fileitem.filename:
   open(os.getcwd() + "/" + os.getenv('DOCUMENT_ROOT') + "/" + whereto + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + "/" + os.getenv('DOCUMENT_ROOT') + "/" + whereto
else:
   message = 'Uploading Failed'

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Post Response</title>")
print("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/form.css\">")
print("</head>")
print("<body>")
print("<div class=\"form\">")
print("<div class=\"title\">Upload Attempted</div>")
print("<div class=\"bulletlist\">")
print("<H3> " + message + " </H3>")
print("</div>")
print("</div>")
print("</body>")
print("</html>")
#!/usr/bin/python3

import cgi, os

form = cgi.FieldStorage()

# Get filename here
fileitem = form['fileToUpload']
whereto = form['whereToUpload'].value

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

# print("Content-Type: text/html;charset=utf-8")
# print ("Content-type:text/html\r\n")
print("<H1> " + message + " </H1>")
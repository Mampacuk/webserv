#!/usr/bin/python3

import sys
import cgi, os

form = cgi.FieldStorage()

stdout_copy = sys.stdout

sys.stdout = sys.stderr

cgi.print_environ()
cgi.print_directory()
cgi.print_arguments()
cgi.print_form(form)
cgi.print_exception()
cgi.print_environ_usage()

sys.stdout = stdout_copy

# Get filename here
fileitem = form['fileToUpload']

# Test if the file was uploaded
if fileitem.filename:
   open(os.getcwd() + '/cgi-bin/tmp/' + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + '/cgi-bin/tmp'
else:
   message = 'Uploading Failed'

# print("Content-Type: text/html;charset=utf-8")
# print ("Content-type:text/html\r\n")
print("<H1> " + message + " </H1>")
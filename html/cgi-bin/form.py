#!/usr/bin/python3

import cgi

print("Content-Type: text/html\r")    # HTML is following
print("\r")                             # blank line, end of headers

form = cgi.FieldStorage() 

firstname = form.getvalue('firstname')
lastname  = form.getvalue('lastname')
email = form.getvalue('email')

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Post Response</title>")
print("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/form.css\">")
print("</head>")
print("<body>")
print("<div class=\"form\">")
print("<div class=\"title\">Thanks for submission!</div>")
print("<div class=\"subtitle\">We've got your info.</div>")
print("<div class=\"bulletlist\">")
print("<ul>")
print("<li>First Name: %s</li>" % (firstname))
print("<li>Last Name: %s</li>" % (lastname))
print("<li>Email: %s</li>" % (email))
print("</ul>")
print("</div>")
print("</div>")
print("</body>")
print("</html>")

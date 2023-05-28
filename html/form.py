#!/usr/bin/env python

import cgi

import os

# printing environment variables
print(os.environ)

# HTML form
print("<html><body>")

# Process form submission
form = cgi.FieldStorage()
if 'name' in form and 'email' in form:
    name = form['name'].value
    email = form['email'].value
    print("<h2>Thank you for submitting the form!</h2><p>Name:")
    print(name)
    print("</p><p>Email:")
    print(email)
    print("</p>")

print("</body></html>")
#!/usr/local/bin/python3

import sys
import cgi
# import cgitb
# cgitb.enable()
# HTML form
print("<html><body>")


# Process form submission
form = cgi.FieldStorage()

cgi.print_environ()
cgi.print_directory()
cgi.print_arguments()
cgi.print_form(form)
cgi.print_exception()
cgi.print_environ_usage()

if 'name' in form and 'email' in form:
    name = form['name'].value
    email = form['email'].value
    print("<h2>Thank you for submitting the form!</h2><p>Name:")
    print(name)
    print("</p><p>Email:")
    print(email)
    print("</p>")
    
    # print("<p>")
    # import os
    # print(os.environ)
    # print("</p>")
else:
    # import os
    # print(os.environ)
    print("<p>Form submission failure. :(</p>")

print("</body></html>")

# import sys
# import os
# import urllib.parse

# # Read raw request data from stdin
# content_length = int(os.environ.get('CONTENT_LENGTH', 0))
# raw_data = sys.stdin.buffer.read(content_length)

# # Decode the data
# decoded_data = urllib.parse.unquote_plus(raw_data.decode('utf-8'))

# # Parse the decoded data as a query string
# form_data = urllib.parse.parse_qs(decoded_data)

# print("<html><body>")
# print("<p>")
# print(form_data)
# print("</p>")

# # Process form submission
# if 'name' in form_data and 'email' in form_data:
#     name = form_data['name'][0]
#     email = form_data['email'][0]
#     # print("Content-type: text/html\r\n\r\n")
#     print("<h2>Thank you for submitting the form!</h2><p>Name: {0}</p><p>Email: {1}</p>".format(name, email))
# else:
#     # print("Content-type: text/html\r\n\r\n")
#     print("<html><body>")
#     print("<p>Form submission failure. :(</p>")

# print("</body></html>")
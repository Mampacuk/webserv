#!/usr/bin/python3

# CGI处理模块
import cgi, cgitb 

# 创建 FieldStorage 的实例化
form = cgi.FieldStorage() 

# 获取数据
site_name = form.getvalue('name')
site_url  = form.getvalue('url')

print ("<html>")
print ("<head>")
print ("<meta charset=\"utf-8\">")
print ("<title>CGI test</title>")
print ("</head>")
print ("<body>")
print ("<h2>site name %s: site_url %s</h2>" % (site_name, site_url))
print ("</body>")
print ("</html>")

# import cgi
# # import cgitb
# # cgitb.enable()
# # HTML form
# print("<html><body>")


# # Process form submission
# form = cgi.FieldStorage()

# cgi.print_environ()
# cgi.print_directory()
# cgi.print_arguments()
# cgi.print_form(form)
# cgi.print_exception()
# cgi.print_environ_usage()

# if 'name' in form and 'email' in form:
#     name = form['name'].value
#     email = form['email'].value
#     print("<h2>Thank you for submitting the form!</h2><p>Name:")
#     print(name)
#     print("</p><p>Email:")
#     print(email)
#     print("</p>")
    
#     # print("<p>")
#     # import os
#     # print(os.environ)
#     # print("</p>")
# else:
#     # import os
#     # print(os.environ)
#     print("<p>Form submission failure. :(</p>")

# print("</body></html>")

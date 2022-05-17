#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb

# Create instance of FieldStorage 
cgitb.enable()
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

print("Content-type:text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<title>Python CGI Program</title>")
print("</head>")
print("<body>")
print(f"<h2>Hello {first_name} {last_name} </h2> ")
print("</body>")
print("</html>")

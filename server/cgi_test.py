#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb

# Create instance of FieldStorage 
cgitb.enable()
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

with open("cgi_serv_communication_file.txt", "w") as response_file:
    # print("Content-type:text/html\r\n\r\n", file=response_file)
    print("<html>", file=response_file)
    print("<head>", file=response_file)
    print("<title>Python CGI Program</title>", file=response_file)
    print("</head>", file=response_file)
    print("<body>", file=response_file)
    print(f"<h2>Hello {first_name} {last_name} </h2> ", file=response_file)
    print("</body>", file=response_file)
    print("</html>", file=response_file)
exit(0)

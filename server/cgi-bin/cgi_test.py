#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb, os
import fileinput

# Create instance of FieldStorage 
cgitb.enable()
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('first_name')
field_of_study = form.getvalue('field_of_study')

print(first_name)
print(field_of_study)

with open("cgi-bin/cgi_serv_communication_file.txt", "w") as response_file:
    with open("/Users/noufel/Documents/Learning/Programming/projets-42/webserv/server/www/noufel_website/index.html", 'r') as original:
        data = original.read()
        data = data.replace('Noufel', first_name)
        data = data.replace('Computer Science', field_of_study)
    print(data)
    print(data, file=response_file)
    # print("<html>", file=response_file)
    # print("<head>", file=response_file)
    # print("<title>Python CGI Program</title>", file=response_file)
    # print("</head>", file=response_file)
    # print("<body>", file=response_file)
    # print(f"<h2>Hello {first_name} {last_name} </h2> ", file=response_file)
    # print("</body>", file=response_file)
    # print("</html>", file=response_file)
exit(0)

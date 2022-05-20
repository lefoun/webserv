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
cgi.print_environ_usage()

path_info = os.environ['PATH_INFO']
request_method = os.environ['REQUEST_METHOD']

if "<script>" in first_name:
    first_name = "Kevin Mitnick"
if "<script>" in field_of_study:
    field_of_study = "as a Script Kiddy"

print(first_name)
print(field_of_study)

with open("cgi-bin/cgi_serv_communication_file.txt", "w") as response_file:
    with open("/Users/noufel/Documents/Learning/Programming/projets-42/webserv/server/www/noufel_website/index.html", 'r') as original:
        data = original.read()
        if first_name:
            data = data.replace('Person', first_name)
        if field_of_study:
            data = data.replace('Computer Science', field_of_study)
    # print(data)
    print(data, file=response_file)
exit(0)

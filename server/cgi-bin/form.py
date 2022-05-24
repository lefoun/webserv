#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb, os, sys
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

file_name = "cgi-bin/cgi_serv_communication_file.txt"
cookie = os.environ['HTTP_COOKIE']
if cookie:
    file_name = "cgi-bin/cookies/" + cookie + "_dir/" + cookie
    print("file_name is a Cookie " + os.environ['HTTP_COOKIE'])
    os.makedirs("cgi-bin/cookies/" + cookie + "_dir/", exist_ok=True)
with open(file_name, "w") as response_file:
    with open("cgi-bin/allin.html", 'r') as original:
        data = original.read()
        if first_name:
            data = data.replace('Person', first_name)
        if field_of_study:
            data = data.replace('Computer Science', field_of_study)
    # print(data)
    print(data, file=response_file)
exit(0)

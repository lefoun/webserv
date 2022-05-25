#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb, os, sys
import fileinput
import secrets
from datetime import datetime


def construct_response(return_code, body, cookie, set_cookie):
    response = "HTTP/1.1 " + str(return_code)
    if return_code == 500:
        return response
    time = datetime.today().strftime("%a, %d %b %Y %H:%M:%S GMT")
    response += "\r\ncontent-type: text/html"
    response += "\r\ncontent-length: " + str(len(body))
    response += "\r\ndate: " + time
    if set_cookie:
        response += "\r\nSet-Cookie: tracking-cookie=" + str(cookie) + "\r\n"
    response += "\r\n"
    print("This is header inside CGI: " + response)
    response += body
    return response

if __name__ == '__main__':
    # Create instance of FieldStorage 
    cgitb.enable()
    form = cgi.FieldStorage() 

    # Get CGI environment variables
    path_info = os.environ['PATH_INFO']
    request_method = os.environ['REQUEST_METHOD']
    cookie = os.environ['HTTP_COOKIE']

    # Get data from fields
    first_name = form.getvalue('first_name')
    field_of_study = form.getvalue('field_of_study')

    # Check if first_name and field of study exist
    if first_name is None:
        first_name = ""
    if field_of_study is None:
        field_of_study = ""

    # "Sanitize" script
    if "<script>" in first_name:
        first_name = "Kevin Mitnick"
    if "<script>" in field_of_study:
        field_of_study = "as a Script Kiddy"

    # default filename
    file_name = ""
    default_file = "cgi-bin/cgi_serv_communication_file.txt"

    # If we didn't find a cookie we generate a cookie
    set_cookie = False
    if cookie is None or cookie == "":
        cookie = secrets.token_hex(nbytes=16)
        set_cookie = True
    if cookie:
        file_name = "cgi-bin/cookies/" + cookie + "_form" 
        print("file_name is a Cookie " + os.environ['HTTP_COOKIE'])
    with open(file_name, "w") as response_file:
        with open("cgi-bin/allin.html", 'r') as original:
            data = original.read()
            if first_name:
                data = data.replace('Person', first_name)
            if field_of_study:
                data = data.replace('Computer Science', field_of_study)
        data = construct_response("200 OK", data, cookie, set_cookie)
        print(data, file=response_file)
    with open(default_file, 'w') as file:
        print(data, file=file)
    exit(0)

#!/Users/noufel/anaconda3/bin/python3
# a#!/usr/bin/python3.9
# c!/Library/Frameworks/Python.framework/Versions/3.9/bin/python3.9
# Import modules for CGI handling
import cgi, cgitb, os, sys
import fileinput
import secrets
from datetime import datetime, timedelta


def construct_response(return_code, body, cookie, set_cookie):
    is_chunked = False
    response = "HTTP/1.1 " + str(return_code)
    if return_code == 500:
        return response
    time = datetime.today().strftime("%a, %d %b %Y %H:%M:%S GMT")
    response += "\r\ncontent-type: text/html"
    if len(body) < BUFFER_SIZE:
        response += "\r\ncontent-length: " + str(len(body))
    response += "\r\ndate: " + time
    if len(body) >= BUFFER_SIZE:
        is_chunked = True
        response += "\r\nTransfer-Encoding: chunked"
    if set_cookie:
        # cookie_date = datetime.today() + timedelta(days=365)
        response += "\r\nSet-Cookie: session-cookie=" + str(cookie) + "; Domain=localhost; Path=/noufel_website; SameSite=Lax" + "\r\n"
    response += "\r\n"
    response += "\r\n"
    response += body
    return response

if __name__ == '__main__':
    # Create instance of FieldStorage
    cgitb.enable()
    form = cgi.FieldStorage()

    # Get CGI environment variables
    request_method = os.environ['REQUEST_METHOD']
    cookie = os.environ['SESSION_COOKIE']
    BUFFER_SIZE = os.environ['BUFFER_SIZE']
    if BUFFER_SIZE is None or BUFFER_SIZE == "":
        BUFFER_SIZE = 4096
    else:
        BUFFER_SIZE = int(BUFFER_SIZE)
    # Get data from fields
    first_name = form.getvalue('first_name')
    field_of_study = form.getvalue('field_of_study')

    # Check if first_name and field of study exist
    if first_name is None:
        first_name = "No One"
    if field_of_study is None:
        field_of_study = "Nothing"

    # "Sanitize" script
    if "<script>" in first_name:
        first_name = "Kevin Mitnick"
    if "<script>" in field_of_study:
        field_of_study = "as a Script Kiddy"

    # default filename
    file_name = ""
    default_file = "cgi-bin/cgi_serv_communication_file.txt"

    set_cookie = False
    # if Cookie not set
    if cookie is None or cookie == "":
        cookie = secrets.token_hex(nbytes=16)
        set_cookie = True
    else:
        set_cookie = False #we don't need to set a cookie we already have one!

    file_name = "cgi-bin/cookies/" + cookie + "_form"
        # print("file_name is a Cookie " + os.environ['HTTP_COOKIE'])
    with open(file_name, "w") as response_file:
        if set_cookie:
            with open("cgi-bin/form_response.html", 'r') as original:
                data = original.read()
                if first_name:
                    data = data.replace('Person', first_name)
                if field_of_study:
                    data = data.replace('Computer Science', field_of_study)
        else:
            data = response_file.read()
        data = construct_response("200 OK", data, cookie, set_cookie)
        print(data, file=response_file)
    with open(default_file, 'w') as file:
        print(data, file=file)
    sys.exit(0)

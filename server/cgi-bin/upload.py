#!/usr/bin/python3

import cgi, os, cgitb

print('hello world')
cgitb.enable()
form = cgi.FieldStorage()

if 'file_item' in form:
    file_item = form['file_name']
else:
    print('CGI Error no file_name in form')
cookie = os.environ['HTTP_COOKIE']
if cookie:
    path = cookie + "_dir/" + cookie
    os.makedirs("cgi-bin/cookies/" + cookie + "_dir/", exist_ok=True)
    print("path is a cookie in upload.py {path}")
else:
    path = ''
    print("path is {path}")

if file_item.filename:
    striped_name = os.path.basename(file_item.filename)
    print("This is stripped name {striped_name}")
    with open('cgi-bin/cookies/' + path + 'uploaded', 'wb') as file:
        file.write(file_item.file.read())
        print('File successfully loaded')
else:
    print('Failed to find file\n')

exit(0)


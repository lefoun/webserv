# The project
- Webserv is a school project in which we create a small webserver (from scratch written in c++) that's compatible with http 1.1 using unix family of networking functions (select(), socket(), listen(), bind(), accept(), etc.)

- You can clone it and play with it if you need a small toy webserver!

### Webserv
- Parse a config file (similiar to the one you use on Nginx).
- Create a webserver with the config file you specified (ports, ip addresses, error files, paths, root, rewrites and redirects, multi-server settings, etc.).
- Listen on the ips/ports you specified.
- Parse the request using a state machine.
- Determine if the request is a valid request (multi-part requests are also handled).
- Serve the static files or forward the request to the CGI (PHP and Python).
- Handle cookies.

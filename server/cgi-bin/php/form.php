<?php

function construct_response($return_code, $body, $cookie, $set_cookie)
{
	date_default_timezone_set('GMT');
	$response = 'HTTP/1.1 ' . $return_code . '\r\n';
	if ($return_code == 500)
		return $response;
	$time = date('D, d M Y H:i:s T');
	echo $time;
	$response .= 'content-type: text/html\r\n';
	$response .= 'date: ' . $time . '\r\n';
	if ($set_cookie == 1)
		$response .= 'set-cookie: tracking-cookie-php=' . $cookie . '\r\n';
	$response .= '\r\n';
	$response .= $body;
	return $response;
}

$path = $_SERVER['PATH_INFO'];
$request_method = $_SERVER['REQUEST_METHOD'];
$first_name = form_get_field('first_name');
$field_of_study = form_get_field('field_of_study');
$default_file = 'cgi-bin/cgi_serv_communication_file.txt';
$cookie;
$set_cookie = 0;
if (!$cookie)
{

	$token = bin2hex(openssl_random_pseudo_bytes(16));
	//token generated
	echo "The generated cookie is : " + $token;
	setcookie('tracking-cookie-php', $token);
}
    // # default filename
    // file_name = ""
    // default_file = "cgi-bin/cgi_serv_communication_file.txt"

    // # If we didn't find a cookie we generate a cookie
    // print(os.getcwd())
    // cookie = None
    // set_cookie = False
    // if cookie is None or cookie == "":
    //     cookie = secrets.token_hex(nbytes=16)
    //     set_cookie = True
    // if cookie:
    //     file_name = "cgi-bin/cookies/" + cookie + "_form"
    //     print("file_name is a Cookie " + cookie)
    // with open(file_name, "w+") as response_file:
    //     with open("cgi-bin/allin.html", 'r') as original:
    //         data = original.read()
    //         if first_name:
    //             data = data.replace('Person', first_name)
    //         if field_of_study:
    //             data = data.replace('Computer Science', field_of_study)
    //     data = construct_response("200 OK", data, cookie, set_cookie)
    //     print(data, file=response_file)
    // with open(default_file, 'w') as file:
    //     print(data, file=file)
    // exit(0)
?>

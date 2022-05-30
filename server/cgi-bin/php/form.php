#!/usr/bin/php-cgi
<?php
	echo $_SERVER['REQUEST_METHOD'] . "\n";
	echo "Variable :" . $_POST['first_name'] . "\n";
	echo "Variable :" . $_POST['last_name'] . "\n";
	echo "Variable :" . $_POST['org'] . "\n";
	echo "Variable :" . $_POST['browser'] . "\n";
	echo "Variable :" . $_POST['email'] . "\n";
	if (!isset($_SERVER['REQUEST_METHOD'])) {
		$status = "HTTP/1.1 500 Internal Server Error";
		$rep = "<!DOCTYPE html><html><title>500 Internal Server Error</title><h1>Internal Server Error</h1></html>";
	}
	else if ($_SERVER['REQUEST_METHOD'] == 'POST'
		&& isset($_POST['first_name'])
		&& isset($_POST['last_name'])
		&& isset($_POST['email'])
		&&  isset($_POST['org'])
		&& isset($_POST['browser'])) {
		$first_name = $_POST['first_name'];
		$last_name = $_POST['last_name'];
		$email = $_POST['email'];
		$org = $_POST['org'];
		$browser = $_POST['browser'];
		$status = "HTTP/1.1 200 OK\r\n";
		echo $rep = "<!DOCTYPE html><html><p>Thank to have complete the survey. The information about you :</p><p><b>First name:</b> $first_name</p><p><b>Last name:</b> $last_name</p><p><b>Email:</b> $email</p><p><p><b>Organization:</b> $org</p><p><b>Browser:</b> $browser</p></html>";
	}
	else if ($_SERVER['REQUEST_METHOD'] == 'GET'
		&& isset($_GET['first_name'])
		&& isset($_GET['last_name'])
		&& isset($_GET['email'])
		&& isset($_GET['org'])
		&& isset($_GET['browser'])) {
		$first_name = $_GET['first_name'];
		$last_name = $_GET['last_name'];
		$email = $_GET['email'];
		$org = $_GET['org'];
		$browser = $_GET['browser'];
		$status = "HTTP/1.1 200 OK\r\n";
		$rep = "<!DOCTYPE html><html><p>Thank to have complete the survey. The information about you :</p><p><b>First name:</b> $first_name</p><p><b>Last name:</b> $last_name</p><p><b>Email:</b> $email</p><p><b>Organization:</b> $org</p><p><b>Browser:</b> $browser</p></html>";
	}
	else if ($_SERVER["REQUEST_METHOD"] != "GET" && $_SERVER["REQUEST_METHOD"] != "POST")
	{
		$status = "HTTP/1.1 405 Method Not Allowed";
		$rep = "<!DOCTYPE html><html><title>405 Method Not Allowed</title><h1>Method Not Allowed</h1></html>";
	}
	else
	{
		$status = "HTTP/1.1 400 Bad Request\r\n";
		$rep = "<!DOCTYPE html><html><title>400 Bad request</title><p>Sorry, it seems that there is an error. Please try again later.</p></html>";
	}
	$full_rep = $status . "Content-Type: text/html\r\n" . "Date: " . gmdate("D, d M Y H:i:s", time())." GMT\r\n" . "Content-Length: " . strlen($rep) . "\r\n\r\n\n" . $rep;
	$file = fopen("../cgi_serv_communication_file.txt", "w");
	fwrite($file, $full_rep);
	fclose($file);
	echo $full_rep;
	exit('Terminating strcipt');
	echo 'after exit';
?>

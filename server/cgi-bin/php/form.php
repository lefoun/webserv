#!/usr/bin/php-cgi
<!--
<html>
<h1>Thanks for filling out the survey</h1>
We will record your information as follows:
<p>
<b>Name:</b> <php echo $_POST["first_name"]; ?><br>
<b>Last Name:</b> <php echo $_POST["last_name"]; ?><br>
<b>Organization:</b> <php echo $_POST["org"]; ?><br>
<b>Email:</b> <php echo $_POST["email"]; ?><br>
<b>The browser you used:</b> <php echo $_POST["browser"]; ?><br>
<b>Bye !</b> -->


<?php
	echo "In php form.php";
	$response = "<html>\n<h1>Thanks for filling out the survey</h1>\nWe will record your information as follows:<p>\n<b>Name:</b>";
	$response .= $_POST["first_name"];
	$response .= "\n<br><b>Last Name:</b> ";
	$response .= $_POST["last_name"];
	$response .= "\n<br><b>Organization:</b> ";
	$response .= $_POST["org"];
	$response .= "\n<b>Email:</b> ";
	$response .= $_POST["email"];
	$response .= "\n<br><b>The browser you used:</b> ";
	$response .= $_POST["browser"];
	$response .= "\n<br><b>Bye !</b>";
	$file = fopen("../php.txt", "w");
	$file = fwrite($file, $response);
	fclose($file);
?>

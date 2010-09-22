<?php
include('common.php');
$nick = $_POST["nick"];
if (ctype_alpha($nick)) {
	global $username, $password, $database;
	mysql_connect(localhost,$username,$password);
	@mysql_select_db($database) or die( "Unable to select database");

	$p = new Player($nick, $INITRANK);
	$query = $p->getSQL();
	mysql_query($query);
	mysql_close();
}
header("Location: $WEBHOME");
exit;
?>

<?php
include('common.php');
$p1name = $_POST["p1"];
$p2name = $_POST["p2"];
$p1rank = $_POST["p1score"];
$p2rank = $_POST["p2score"];

if (is_numeric($p1rank) and is_numeric($p2rank) and ctype_alpha($p1name) and ctype_alpha($p2name)) {
	global $username, $password, $database, $p1name, $p2name, $p1rank, $p2rank;
	$people = load_people();
	$p1 = lookup_player($people, $p1name);
	$p2 = lookup_player($people, $p2name);
	if ($p1 != null and $p2 != null) {
		$diff = calculate_score($p1, $p2, $p1rank, $p2rank);
		$match = new Match($p1, $p2, $p1rank, $p2rank, $diff);
		update_people($people);
		mysql_connect(localhost,$username,$password);
		@mysql_select_db($database) or die( "Unable to select database");
		$query = $match->getSQL();
		mysql_query($query);
		mysql_close();
	}
}
header("Location: $WEBHOME");
exit;
?>

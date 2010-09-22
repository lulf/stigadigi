<?php
$INITRANK = 1000;
$username = "";
$password = "";
$database = "";
$WEBHOME = "";

class Player
{
	private $nick;
	private $rank;
	private $wins;
	private $games;
	function __construct($mynick, $myrank) {
		$this->nick = $mynick;
		$this->rank = $myrank;
		$this->games = 0;
		$this->wins = 0;
	}
	public function getNick() {
		return $this->nick;
	}
	public function setNick($newnick) {
		$this->nick = $newnick;
	}
	public function setRank($newrank) {
		$this->rank = $newrank;
	}
	public function setGames($newgames) {
		$this->games = $newgames;
	}
	public function setWins($newwins) {
		$this->wins = $newwins;
	}
	public function getRank() {
		return $this->rank;
	}
	public function getWins() {
		return $this->wins;
	}
	public function getGames() {
		return $this->games;
	}
	public function getLosses() {
		return ($this->games - $this->wins);
	}
	public function getSQL() {
		return "INSERT INTO person VALUES(\"$this->nick\", \"$this->rank\")";
	}
}

class Match
{
	private $yellow;
	private $red;
	private $score_yellow;
	private $score_red;
	private $diff;

	function __construct($p1, $p2, $score_yellow, $score_red, $diff) {
		$this->yellow = $p1;	
		$this->red = $p2;	
		$this->score_yellow = $score_yellow;
		$this->score_red = $score_red;
		$this->diff = $diff;
	}

	function getSQL() {
		$p1nick = $this->yellow->getNick();
		$p2nick = $this->red->getNick();
		$score_yellow = $this->score_yellow;
		$score_red = $this->score_red;
		$diff = $this->diff;
		return "INSERT INTO game(yellow, red, score_yellow, score_red, points_changed) VALUES('$p1nick', '$p2nick', '$score_yellow', '$score_red', '$diff')";
	}

	function printHTML() {
		print "<tr><td>";
		print $this->yellow->getNick();
		print "</td><td>";
		print $this->score_yellow;
		print "</td><td>";
		if ($this->score_yellow > $this->score_red) {
			print $this->diff;
		} else {
			print -$this->diff;
		}
		print "</td><td>";
		print $this->red->getNick();
		print "</td><td>";
		print $this->score_red;
		print "</td><td>";
		if ($this->score_red > $this->score_yellow) {
			print $this->diff;
		} else {
			print -$this->diff;
		}
		print "</td></tr>";
		
	}
}

function load_people() {
	global $username, $password, $database;
	mysql_connect(localhost, $username, $password);
	@mysql_select_db($database) or die( "Unable to select database");

	$query = "SELECT * FROM person ORDER BY rank DESC";
	$result = mysql_query($query);
	$num = mysql_numrows($result);
	$i=0;
	$people = array();
	while ($i < $num) {
		$nick = mysql_result($result, $i, "nick");
		$rank = mysql_result($result, $i, "rank");
		$people[$i] = new Player($nick, $rank);
		$i++;
	}
	# Calculate the number of wins and losses
	foreach ($people as $p) {
		$nick = $p->getNick();
		$query_games = "SELECT COUNT(*) AS games FROM game WHERE yellow='$nick' OR red='$nick'";
		$query_wins = "SELECT COUNT(*) AS wins FROM game WHERE (score_yellow > score_red AND yellow='$nick') OR (score_red > score_yellow AND red='$nick')";
		$result = mysql_query($query_games);
		$num = mysql_numrows($result);
		if ($num > 0) {
			$games = mysql_result($result, 0, "games");
			$p->setGames($games);
		}
		$result = mysql_query($query_wins);
		$num = mysql_numrows($result);
		if ($num > 0) {
			$wins = mysql_result($result, 0, "wins");
			$p->setWins($wins);
		}

	}
	mysql_close();
	return $people;
}

function lookup_player($players, $nick) {
	foreach ($players as $p) {
		if (strcmp($p->getNick(), $nick) == 0)
			return $p;
	}
	return null;
}

function load_matches($players) {
	global $username, $password, $database;
	mysql_connect(localhost, $username, $password);
	@mysql_select_db($database) or die( "Unable to select database");

	$query = "SELECT * FROM game";
	$result = mysql_query($query);
	$num = mysql_numrows($result);
	$i=0;
	$matches = array();
	while ($i < $num) {
		$yellow = mysql_result($result, $i, "yellow");
		$red = mysql_result($result, $i, "red");
		$score_yellow = mysql_result($result, $i, "score_yellow");
		$score_red = mysql_result($result, $i, "score_red");
		$diff = mysql_result($result, $i, "points_changed");
		$p1 = lookup_player($players, $yellow);
		$p2 = lookup_player($players, $red);
		if ($p1 != null and $p2 != null) {
			$matches[$i] = new Match($p1, $p2, $score_yellow, $score_red, $diff);
			$i++;
		}
	}
	mysql_close();
	return $matches;
}

function update_people($players) {
	global $username, $password, $database;
	mysql_connect(localhost, $username, $password);
	@mysql_select_db($database) or die( "Unable to select database");

	foreach ($players as $player) {
		$rank = $player->getRank();
		$nick = $player->getNick();
		$query = "UPDATE person SET rank=$rank WHERE nick='$nick'";
		mysql_query($query);
	}
	mysql_close();
}


function calculate_score($p1, $p2, $score_yellow, $score_red) {
	$p1new = $p2new = $diff = $offset = 0;

	if ($score_yellow > $score_red) {
		$diff = ((($score_yellow + 1) / ($score_red + 1))) - ($p1->getRank() / $p2->getRank());
		$offset = round($diff * 50);
		$p1new = $p1->getRank() + $offset;
		$p2new = $p2->getRank() - $offset;
	} else if ($score_yellow < $score_red) {
		$diff = (($score_red + 1) * 1.0 / ($score_yellow + 1)) - ($p2->getRank() / $p1->getRank());
		$offset = round($diff * 50);
		$p1new = $p1->getRank() - $offset;
		$p2new = $p2->getRank() + $offset;
	}
	$p1->setRank($p1new);
	$p2->setRank($p2new);
	return $offset;
}


function print_people($people) {
	print "<h3>Participants:</h3>";
	print "<table border=\"1\"><tr><th>Rank</th><th>Nick</th><th>Games</th><th>Wins</th><th>Losses</th></tr>\n";
	foreach ($people as $p) {
		$nick = $p->getNick();
		$rank = $p->getRank();
		$games = $p->getGames();
		$wins = $p->getWins();
		$losses = $p->getLosses();
		print "<tr><td>$rank</td><td>$nick</td><td>$games</td><td>$wins</td><td>$losses</td></tr>\n";
	}
	print "</table>\n";
}

function print_matches($matches) {
	print "<h3>Last matches:</h3>";
	print "<table border=\"1\"><tr><th>Yellow</th><th>Yellow Score</th><th>Points<th>Red</th><th>Red Score</th><th>Points</th></tr>";
	foreach ($matches as $match) {
		$match->printHTML();
	}
	print "</table>";
}

function print_header() {
	print '<html><head><title>Icehockey o\'matic</title></head><body>';
	print '<h1>Icehockey o\'matic</h1>';
	print '<a href="index.php">List games</a> | ';
	print '<a href="highscore.php">List highscore</a> | ';
	print '<a href="register_match.php">Register match</a> | ';
	print '<a href="register_player.php">Register player</a>';
	print '<br><hr>';
}

function print_footer() {
	print '</body></html>';
}
?>

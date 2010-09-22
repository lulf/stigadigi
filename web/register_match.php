<?php
include('common.php');
print_header();
print '<h3>Register match:</h3><form action="insert.php" method="post"> Yellow/blue Nick: <input type="text" name="p1"> Score: <input type="text" name="p1score"><br> Red/white Nick:&nbsp;&nbsp;<input type="text" name="p2"> Score: <input type="text" name="p2score"><br> <input type="Submit" value="Add"> </form>';
print_footer();
?>

<?php
include('common.php');
print_header();
$people = load_people();
print_people($people);
print_footer();
?>

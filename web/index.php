<?php
include('common.php');
$people = load_people();
$matches = load_matches($people);
print_header();
print_matches($matches);
print_footer();
?>

<?php
///////////////////////////////////////
//                                   //
// index.php - central point in site //
//    (c) Andrew Sidwell, 2002       //
//                                   //
///////////////////////////////////////

// Include the general layout functions
$included = true;
include('layout.php');

// Ensure we always have a page, and style
if (!$page) $page = 'main';
if (!$cstyle) $style = 'clean';
         else $style = $cstyle;

// Check on the setting styles
if (($newstyle == 'sidebar') ||
    ($newstyle == 'clean'))
{
	// Set a cookie for a few years
	setcookie("cstyle", $newstyle, time()+172800000);

	// Set the current style
	$style = $newstyle;
}

// Include stuff from 'extras'
if ($dir = @opendir('extras/'))
{
	while (($file = readdir($dir)) != false)
	{
		if (!is_dir('extras/'.$file)) include('extras/'.$file);
	}
}

// Come up with a page "title" - done in files.php
include('files.php');
$page_okay = (file_exists('content/' .$page) && !is_dir('content/' .$page));
if (!$page_okay) $title = 'page not found';

// Output the header
page_header($title, $page, $pagestyle);

// Include the data
if ($page_okay)
{
	include('content/'.$page);
}
else
{
	echo '<p><b>Sorry, that page was not found.</b></p>';
	echo '<p>You may be looking for a page which does not exist any more; please follow the links on the sidebar.</p>';
}

// Output the footer
page_footer();

?>

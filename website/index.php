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

// Ensure we always have a page. and style
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

// Come up with a page "title"
switch ($page)
{
	case 'main': $title = 'main page';
	             break;

	case 'changes': $title = 'changes';
	                break;

	case 'download': $title = 'download';
	                 break;

	case 'project': $redirect = 'http://developer.berlios.de/project/?group_id=266';
	                break;
}

// Redirect if appropriate
if ($redirect)
{
	// This redirects to another page.
	header('Location: ' .$redirect);
}
else
{
	// Output the header
	page_header($title, $page, $pagestyle);

	// Include the data
	include('content/'.$page);

	// Output the footer
	page_footer();
}

// We are done.

?>

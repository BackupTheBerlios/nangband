<?php
///////////////////////////////////////
//                                   //
// index.php - central point in site //
//    (c) Andrew Sidwell, 2002       //
//                                   //
///////////////////////////////////////

// Ensure we don't have rubbish
if ((($page != 'main') &&
     ($page != 'changes') &&
     ($page != 'download') &&
     ($page != 'project')) ||
     (!$page))
{
	$page = 'main';
}

// Check on the setting styles
if (($newstyle == 'sidebar') ||
    ($newstyle == 'clean'))
{
	// Set a cookie for a few years
	setcookie("style", $newstyle, time()+172800000);

	// Set the current style
	$style = $newstyle;
}

// Check on "current" styles
if ((($style != 'sidebar') &&
     ($style != 'clean')) ||
     (!$style))
{
	$style = 'clean';
}


// Silly.
$access = 'arnold';

// Include the general layout functions
include('layout.php');

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

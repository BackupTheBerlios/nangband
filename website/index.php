<?php
///////////////////////////////////////
//                                   //
// index.php - central point in site //
//    (c) Andrew Sidwell, 2002       //
//                                   //
///////////////////////////////////////

// Include the general layout functions
include('layout.php');

// Ensure we don't have rubbish
if ((($page != 'main') &&
     ($page != 'changes') &&
     ($page != 'download') &&
     ($page != 'project')) ||
     (!$page))
{
	$page = 'main';
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

	case 'project': $redirect = 'http://developer.berlios.de/';
	                break;
}

// Output the header
page_header($title, $page);

// Include the data
include($page);

// Output the footer
page_footer();
?>

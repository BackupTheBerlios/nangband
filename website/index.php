<?php
///////////////////////////////////////
//                                   //
// index.php - central point in site //
//    (c) Andrew Sidwell, 2002       //
//                                   //
///////////////////////////////////////

// Include the general layout functions and file listings
include('sitedata.php');
include('layout.php');

// Ensure we always have a page, and style
if (!$page) $page = 'main';
//if (!$cstyle) $style = $default_style;
//         else $style = $cstyle;

// Check on the setting styles
//if (($newstyle == 'normal') ||
//    ($newstyle == 'plain'))
//{
//	// Set a cookie for a few years
//	setcookie("cstyle", $newstyle, time()+172800000);

//	// Set the current style
//	$style = $newstyle;
//}

$style = 'plain';

// Include stuff from 'extras'
if ($dir = @opendir('extras/'))
{
	while (($file = readdir($dir)) != false)
		if (!is_dir('extras/'.$file))
			include('extras/'.$file);
}

// Check for a filename's validity
if ($titles[$page]) $title = $titles[$page];

// Extract some information
$dpage = $page;

if ($image)
{
	$incdir = 'images/';
	$dpage .= '.desc';
	$title = 'image';
	$image = true;
}
else $incdir = 'content/';

// Check for existance
$okay = (file_exists($incdir.$dpage) && !is_dir($incdir.$dpage)); 
if (file_exists('styles/'.$style.'.common')) include('styles/'.$style.'.common');
if (!$okay) $title = 'page not found';
if (isset($source)) $title = 'source';

// Output the header
page_header($title, $page, $style);

// Include the data
if ($okay) include($incdir.$dpage);
else include('error');

// Output the footer
page_footer();

?>

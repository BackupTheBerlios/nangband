<?php
///////////////////////////////////////
//                                   //
// index.php - central point in site //
//    (c) Andrew Sidwell, 2002       //
//                                   //
///////////////////////////////////////

// Micro-hack to give nice source listings
$included[1] = true;

// Include the general layout functions and file listings
include('sitedata.php');
include('layout.php');

// Ensure we always have a page, and style
if (!$page) $page = 'main';
if (!$cstyle) $style = $default_style;
         else $style = $cstyle;

// Check on the setting styles
if (($newstyle == 'sidebar') ||
    ($newstyle == 'clean') ||
    ($newstyle == 'plain'))
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
if (!$okay) $title = 'page not found';
if (isset($source)) $title = 'source';

// Output the header
page_header($title, $page, $style);

// Include the data
if (isset($source))
{
	echo '<p>The source files used in the nangband site are <a href="?source=layout">layout.php</a>,';
	echo ' <a href="?source=files">files.php</a>, and <a href="?source=true">index.php</a>.  Click ';
	echo 'on one of the files listed above to see it\'s source code.</p>';
	echo '<p>Here is the source:</p>';

	if ($source == 'layout') $source = 'layout.php';
	else if ($source == 'files') $source = 'files.php';
	else $source = 'index.php';

	show_source($source);
}
else if ($okay)
{
	// Include the data
	include($incdir.$dpage);
}
else 
{
	echo '<p><b>Sorry, that page was not found.</b></p>';
	echo '<p>You may be looking for a page which does not exist any more; please follow the links on the sidebar.</p>';
}

// Output the footer
page_footer();

?>

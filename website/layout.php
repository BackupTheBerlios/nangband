<?php
///////////////////////////////////
//                               //
// layout.php - layout functions //
//   (c) Andrew Sidwell, 2002    //
//                               //
///////////////////////////////////

// Sidebar link ends and beginnings
$sidebar_links_start = '<tr><td bgcolor="#dddddd" align="center">';
$sidebar_links_end   = '</td></tr>';

// Output all the links using flexible formatting.
function do_links($has_pipes = true, $start = 0, $end = 0)
{
	global $titles, $pages, $no_pages, $page;

	// List all the pages
	for ($i = 0; $i < $no_pages; $i++)
	{
		if ($start) echo $start;
		if ($page == $pages[$i]) echo $titles[$pages[$i]];
		else echo '<a href="?page=' .$pages[$i]. '">'.$titles[$pages[$i]].'</a>';
		if ($has_pipes && ($i < ($no_pages - 1))) echo ' | ';
		if ($end) echo $end;
	}

	return;
}

function do_styles($has_pipes = true, $ucfirst = 0)
{
	global $styles, $no_styles, $showimage, $page;

	for ($i = 0; $i < $no_styles; $i++)
	{
		if ($style == $styles[$i])
		{
			if ($ucfirst == false) echo $styles[$i];
			else echo ucfirst($styles[$i]);
		}
		else
		{
			echo '<a href="?page='.$page.'&amp;';
			if ($showimage) echo 'showimage=true&amp;';
			echo 'newstyle='.$styles[$i].'">';
			if ($ucfirst == false) echo $styles[$i];
			else echo ucfirst($styles[$i]);
			echo '</a>';			
		}

		// Add the "pipes"
		if ($has_pipes && ($i < ($no_styles - 1))) echo ' | ';
	}

	return;
}

function page_header($title, $type)
{
	global $style, $sidebar_links_start, $sidebar_links_end;

	echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">';
	echo '<html>';
	echo '<head>';
	echo '<title>nangband - ' . $title . '</title>';
	echo '<link rel="stylesheet" rev="content" type="text/css" href="styles/'.$style.'.css">';
	echo '<meta name="description" content="A site about Nangband, an Angband variant.">';
	echo '</head>';

	// Include the style header (if it exists)
	if (file_exists('styles/'.$style.'.head')) include('styles/'.$style.'.head');
	else echo '<body>';

	return;
}

function page_footer()
{
	global $style;

	// Include the style header (if it exists)
	if (file_exists('styles/'.$style.'.foot')) include('styles/'.$style.'.foot');

	echo '</body></html>';

	return;
}

// Redirect to the "real" source lister if we're not included
if (!$included[1]) header('Location: /?source=layout');

?>

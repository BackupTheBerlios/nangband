<?php
///////////////////////////////////
//                               //
// layout.php - layout functions //
//   (c) Andrew Sidwell, 2002    //
//                               //
///////////////////////////////////

// "Make" a "link" (and "do" not "print" it)
function make_link($data)
{
	$link = '<a href="?page=';
	$link .= ($data['image'] ? $data['image'] : $data['page']);
	if ($data['image']) $link .= '&amp;image=true';
	if ($data['style']) $link .= '&amp;newstyle=' .$data['style'];
	$link .= '">' .$data['text']. '</a>'; 

	return ($link);
}

// Output all the links using flexible formatting.
function do_links($has_pipes = true, $uc = false, $start = 0, $end = 0, $sep = ' | ')
{
	global $titles, $pages, $page;

	$x = count($pages);
	for ($i = 0; $i < $x; ++$i)
	{
		if ($start) echo $start;

		if ($uc == false) $echo_text = $titles[$pages[$i]];
		else $echo_text = ucwords($titles[$pages[$i]]);

		if ($page != $pages[$i])
			echo make_link(array('page' => $pages[$i], 'text' => $echo_text));
		else
			echo $echo_text;

		if ($has_pipes && ($i < ($x - 1))) echo $sep;
		if ($end) echo $end;
	}

	return;
}

function do_styles($has_pipes = true, $uc = false, $start = 0, $end = 0, $sep = ' | ')
{
	global $styles, $image, $page, $style;

	$x = count($styles);
	for ($i = 0; $i < $x; ++$i)
	{
		if ($start) echo $start;

		if ($uc == false) $echo_text = $styles[$i];
		else $echo_text = ucwords($styles[$i]);

		if ($style != $styles[$i])
			echo make_link(array('page' => $page, 'image' => $image, 
				'style' => $styles[$i], 'text' => $echo_text));
		else
			echo $echo_text;

		if ($has_pipes && ($i < ($x - 1))) echo $sep;
		if ($end) echo $end;
	}

	return;
}

function page_header($title, $type)
{
	global $style, $site_description, $site_name;

	// Echo the page header
	echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">';
	echo '<html>';
	echo '<head>';
	echo '<title>'.$site_name.' - ' . $title . '</title>';
	echo '<link rel="stylesheet" rev="content" type="text/css" href="styles/'.$style.'.css">';
	echo '<meta name="description" content="'.$site_description.'">';
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

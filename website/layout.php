<?php
///////////////////////////////////
//                               //
// layout.php - layout functions //
//   (c) Andrew Sidwell, 2002    //
//                               //
///////////////////////////////////

// Make sure prying eyes get a nice response ;)
if ($access != 'arnold')
{
	echo '<html><head><title>Access Denied.</title></head><body><h1>Access Denied.</h1></body></html>';
}

function do_links()
{
	echo '[ <a href="?page=main">Main Page</a> | ';
	echo '<a href="?page=changes">Changes</a> | ';
	echo '<a href="?page=download">Download</a> | ';
	echo '<a href="?page=project">Project Page</a> ]';

	return;
}

function do_styles($type)
{
	global $style;

	if ($style != 'sidebar')
	{
		echo '<a href="?page='.$type.'&newstyle=sidebar">Sidebar-thing</a>';
	}
	else
	{
		echo 'Sidebar-thing';
	}

	echo ' | ';

	if ($style != 'clean')
	{
		echo '<a href="?page='.$type.'&newstyle=clean">Clean</a>';
	}
	else
	{
		echo 'Clean';
	}


	return;
}

function page_header($title, $type)
{
	echo '<html>';
	echo '<head>';
	echo '<title>Nangband - ' . $title . '</title>';
	echo '<link rel="stylesheet" rev="content" type="text/css" href="style.css">';
	echo '<meta name="description" content="A site about Nangband, an Angband variant.">';
	echo '</head>';
	echo '<body bgcolor="#ffffee">';
	echo '<table width="95%" align="center" callspacing="1" cellpadding="0" border="0">';
	echo '<tr><td>';
	echo '<table width="100%" align="center" callspacing="0" cellpadding="4" border="0">';
	echo '<tr><td bgcolor="#eeeeee" align="center"><font face="Veranda" color="#555555" size="+2">';

	echo '<b>nangband - '.$title.'</b></font>';

	echo '<br>';
	echo '<font face="Verdana" size="-1" color="#555555">Change style: ';
	do_styles($type);
	echo '</font>';

	echo '<font face="Veranda" color="#555555">';
	do_links();
	echo '</font>';
	echo '</td></tr>';

	echo '<tr><td bgcolor="#dddddd"><font face="Veranda" color="#333333">';

	return;
}

function page_footer()
{
	echo '</font></td></tr>';
 
	echo '<tr><td bgcolor="#eeeeee" align="center">';
	echo '<font face="Veranda" color="#555555">';
	do_links();
	echo '<br>';
	echo '<font size="-1"><div align="center">Material &copy; Nangband Developers. Contact ';
	echo '<a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>';

	echo '</font></td></tr>';
	echo '</table></td></tr></table>';

	echo '</body></html>';

	return;
}

<?php
/*
 * layout.php - page layout functions
 * ----------------------------------
 *
 * started on:		07/03/02
 * by:				takkaria
 */

function page_header($title, $type)
{
	echo '<html>';
	echo '<head>';
	echo '<title>Nangband - ' . $title . '</title>';
	echo '<link rel="stylesheet" rev="content" type="text/css" href="style.css">';
	echo '<meta name="description" content="A site about Nangband, an Angband variant.">';
	echo '</head>';
	echo '<body bgcolor="#000000" text="#ffffff" link="#d090ff" alink="#00ffff" vlink="#77ff44">';
	echo '<h1 align="center"><img src="images/nangband.gif" align="center"><br>'.$title.'</h1>';

	echo '<div align="center">';
	echo '[ <a href="index.php">Main Page</a> | ';
	echo '<a href="changes.php">Changes</a> | ';
	echo '<a href="bugs.php">Bugs</a> | ';
	echo '<a href="download.php">Download</a> | ';
	echo '<a href="http://developer.berlios.de/projects/nangband">Project Page</a> ] ';
	echo '</div>';

	echo '<hr width="450" size="1" align="center">';
	echo '<br';

	return;
}

function page_footer($creators)
{
	echo '<hr width="50%" size="1" align="center">';
	echo '<font size="-1"><div align="center">All material is &copy; ' . $creators . '. Contact ';
	echo '<a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>';
	echo '</div></font>';
	echo '</body></html>';

	return;
}

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
	echo '<head><title>Nangband - ' . $title . '</title></head>';
	echo '<body bgcolor="#ffffff" text="#000000">';
	echo '<h1 align="center">' . $title . '</h1>';
	echo '<div align="center">';
	echo '[ <a href="http://developer.berlios.de/projects/nangband">Berlios Project Page</a> ] ';
	echo '[ <a href="index.php">Main</a> ] ';
	echo '[ <a href="changes.php">Changes</a> ]';
	echo '</div>';
	echo '<hr width="50%" size="1" align="center">';

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

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
	echo '<body bgcolor="#000000" text="#ffffff">';
	echo '<h1 align="center">' . $title . '</h1>';
	echo '<hr width="50%" align="center">';

	return;
}

function page_footer()
{
	echo '<hr width="50%" align="center">';
	echo '<div align="center">Nangband website. Contact ';
	echo '<a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>';
	echo '</div>';
	echo '</body></html>';

	return;
}

<?php
/*
 * layout.php - page layout functions
 * ----------------------------------
 *
 * started on:		07/03/02
 * by:				takkaria
 */

/*if ($access != 'eric')
{
echo '<html><head><title>Access Denied.</title></head><body>Access Denied.</body></html>';
}*/

function do_links()
{
	echo '[ <a href="index.php">Main Page</a> | ';
	echo '<a href="changes.php">Changes</a> | ';
	echo '<a href="download.php">Download</a> | ';
	echo '<a href="http://developer.berlios.de/projects/nangband">Project Page</a> ]';

	return;
}

function page_header($title, $type)
{
	echo '<html>\n';
	echo '<head>\n';
	echo '<title>Nangband - ' . $title . '</title>\n';
/*	echo '<link rel="stylesheet" rev="content" type="text/css" href="style.css">'; */
	echo '<meta name="description" content="A site about Nangband, an Angband variant.">\n';
	echo '</head>\n';
	echo '<body bgcolor="#ffffee">\n';
	echo '<table width="95%" align="center" callspacing="1" cellpadding="0" border="0">\n';
	echo '<tr><td>\n';
	echo '<table width="100%" align="center" callspacing="0" cellpadding="4" border="0">\n';
	echo '<tr><td bgcolor="#eeeeee" align="center"><font face="Veranda" color="#555555" size="+2">';

	echo '<b>Nangband - '.$title.'</b></font>\n';

	echo '<br>\n';
	echo '<font face="Veranda" color="#555555">';
	do_links();
	echo '</font>\n';
	echo '</td></tr>\n';

	echo '<tr><td bgcolor="#dddddd"><font face="Veranda" color="#333333">';

	return;
}

function page_footer($creators)
{
	echo '</font></td></tr>\n';
 
	echo '<tr><td bgcolor="#eeeeee" align="center">';
	echo '<font face="Veranda" color="#555555">';
	do_links();
	echo '<br>';
	echo '<font size="-1"><div align="center">All material is &copy; ' . $creators . '. Contact ';
	echo '<a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>';

	echo '</font></td></tr>\n';
	echo '</table></td></tr></table>\n';

	echo '</body></html>';

	return;
}

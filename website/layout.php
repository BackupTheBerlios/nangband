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

function do_sidebar_links()
{
?>
	<tr><td bgcolor="#bbbbbb" align="center"><a href="?page=main">main page</a></td></tr>
	<tr><td bgcolor="#bbbbbb" align="center"><a href="?page=changes">changes</a></td></tr>
	<tr><td bgcolor="#bbbbbb" align="center"><a href="?page=download">download</a></td></tr>
	<tr><td bgcolor="#bbbbbb" align="center"><a href="?page=project">project page</a></td></tr>
<?php

	return;
}

function do_clean_links()
{
	echo '<a href="?page=main">main page</a> | ';
	echo '<a href="?page=changes">changes</a> | ';
	echo '<a href="?page=download">download</a> | ';
	echo '<a href="?page=project">project page</a>';

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
	global $style;

	echo '<html>';
	echo '<head>';
	echo '<title>nangband - ' . $title . '</title>';
	echo '<link rel="stylesheet" rev="content" type="text/css" href="styles/'.$style.'.css">';
	echo '<meta name="description" content="A site about Nangband, an Angband variant.">';
	echo '</head>';

	if ($style == 'clean')
	{
?>
<body bgcolor="#ffffff">
<table width="95%" align="center" callspacing="1" cellpadding="0" border="0">
	<tr><td>
		<table width="100%" align="center" callspacing="0" cellpadding="4" border="0">
		<tr><td bgcolor="#eeeeee" align="center">
			<font face="Veranda" color="#555555" size="+2">
			<b>nangband - <?php echo $title; ?></b></font>

			<br>
			<font face="Verdana" size="-1" color="#555555">Change style: <?php do_styles($type); ?></font>
			<br>

			<font face="Veranna" color="#555555"><?php do_clean_links(); ?></font>

		</td></tr>
		<tr><td bgcolor="#dddddd"><font face="Veranna" color="#333333">
<?php
	}
	else if ($style == 'sidebar')
	{
?>
<body bgcolor="#ffffff">
<table width="95%" border="0">
	<tr><td valign="top" width="15%">
			<table align="center" cellpadding="20" cellspacing="10" border="0" width="95%">
				<?php do_sidebar_links(); ?>
			</table>
		</td>
		<td valign="top">
			<table width="100%" callspacing="0" cellpadding="4" border="0">
				<tr><td bgcolor="#eeeeee" align="center">
					<font size="+2">nangband - <? echo $title; ?></font>
					<br>
					<font face="Verdana" size="-1" color="#555555">Change style: <?php do_styles($type); ?></font>
				</td></tr>
				<tr>
					<td bgcolor="#dddddd">
<?php
	}

	return;
}

function page_footer()
{
	global $style;

	if ($style == 'clean')
	{
		echo '</font></td></tr>';
 
		echo '<tr><td bgcolor="#eeeeee" align="center">';
		echo '<font face="Veranda" color="#555555">';
		do_clean_links();
		echo '<br>';
		echo '<font size="-1"><div align="center">Material &copy; Nangband Developers. Contact ';
		echo '<a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>';

		echo '</font></td></tr>';
		echo '</table></td></tr></table>';
	}
	else if ($style == 'sidebar')
	{
?>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>

<table align="center" border="0">
	<tr>
		<td>
			<font face="Verdana" color="#555555" size="-1">&copy; Nangband Developers.<br>
Contact <a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a></font>
		</td>
	</tr>
</table>
<?php
	}

	echo '</body></html>';

	return;
}

?>

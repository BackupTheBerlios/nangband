<?php
///////////////////////////////////
//                               //
// layout.php - layout functions //
//   (c) Andrew Sidwell, 2002    //
//                               //
///////////////////////////////////

// Sidebar link ends and beginnings
$sidebar_links_start = '<tr><td bgcolor="#bbbbbb" align="center">';
$sidebar_links_end   = '</td></tr>';

// Output all the links using flexible formatting.
function do_links($start, $end, $has_pipes)
{
        if ($start) echo $start;
        echo '<a href="?page=main">main page</a>';
        if ($has_pipes) echo ' | ';
        if ($end) echo $end;

        if ($start) echo $start;
        echo '<a href="?page=changes">changes</a>';
        if ($has_pipes) echo ' | ';
        if ($end) echo $end;

        if ($start) echo $start;
        echo '<a href="?page=download">download</a>';
        if ($has_pipes) echo ' | ';

        if ($end) echo $end;

        if ($start) echo $start;
        echo '<a href="?page=project">project page</a>';
        if ($end) echo $end;

        return;
}

function do_styles($type)
{
	global $style;

	if ($style != 'sidebar')
	{
		echo '<a href="?page='.$type.'&amp;newstyle=sidebar">Sidebar-thing</a>';
	}
	else
	{
		echo 'Sidebar-thing';
	}

	echo ' | ';

	if ($style != 'clean')
	{
		echo '<a href="?page='.$type.'&amp;newstyle=clean">Clean</a>';
	}
	else
	{
		echo 'Clean';
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

	if ($style == 'clean')
	{
?>
<body bgcolor="#ffffff">
<table width="95%" align="center" cellspacing="1" cellpadding="0" border="0">
	<tr><td>
		<table width="100%" align="center" cellspacing="0" cellpadding="4" border="0">
		<tr><td bgcolor="#eeeeee" align="center">
			<font color="#555555" size="+2"><b>nangband - <?php echo $title; ?></b></font>

			<br>

			<font size="-1" color="#555555">Change style: <?php do_styles($type); ?></font>

			<br>

			<font color="#555555"><?php do_links(0, 0, 1); ?></font>

		</td></tr>
		<tr><td bgcolor="#dddddd">
<?php
	}
	else if ($style == 'sidebar')
	{
?>
<body bgcolor="#ffffff">
<table width="95%" border="0">
	<tr><td valign="top" width="15%">
			<table align="center" cellpadding="20" cellspacing="10" border="0" width="95%">
				<?php do_links($sidebar_links_start, $sidebar_links_end, 0); ?>
			</table>
		</td>
		<td valign="top">
			<table width="100%" cellspacing="0" cellpadding="4" border="0">
				<tr><td bgcolor="#eeeeee" align="center">
					<font size="+2">nangband - <? echo $title; ?></font>
					<br>
					<font size="-1" color="#555555">Change style: <?php do_styles($type); ?></font>
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
		echo '</td></tr>';
 
		echo '<tr><td bgcolor="#eeeeee" align="center">';
		echo '<font color="#555555">';
		do_links(0, 0, 1);
		echo '</font>';
		echo '<br>';
		echo '<div align="center"><font size="-1" color="#555555">';
		echo 'Material &copy; Nangband Developers. Contact ';
		echo '<a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>';

		echo '</font></div></td></tr>';
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
			<font color="#555555" size="-1">&copy; Nangband Developers.<br>
Contact <a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a></font>
		</td>
	</tr>
</table>
<?php
	}

	echo '</body></html>';

	return;
}

// Make sure prying eyes get a nice response :)
if ($included == false)
{
	// Output a nice header
	page_header('Source for layout.php', 'clean');

	// Show the source
	show_source('layout.php');

	// Output a nice footer
	page_footer();
}

?>

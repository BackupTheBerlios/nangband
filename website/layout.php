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
function do_links($start, $end, $has_pipes)
{
	global $titles, $pages, $no_pages;

	for ($i = 0; $i < ($no_pages - 1); $i++)
	{
		if ($start) echo $start;
		echo '<a href="?page=' .$pages[$i]. '">'.$titles[$pages[$i]].'</a>';
		if ($has_pipes) echo ' | ';
		if ($end) echo $end;
	}

	if ($start) echo $start;
	echo '<a href="?page='.$pages[$no_pages-1].'">'.$titles[$pages[$no_pages-1]].'</a>';
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

	echo ' | ';

	if ($style != 'plain')
	{
		echo '<a href="?page='.$type.'&amp;newstyle=plain">Plain</a>';

	}
	else
	{
		echo 'Plain';
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
		<table width="100%" align="center" cellspacing="0" cellpadding="15" border="0">
		<tr><td class="border" bgcolor="#dddddd" align="center">
			<font size="+2"><b>nangband - <?=$title?></b></font>
		</td></tr><tr><td bgcolor="#ffffff" align="center">
			<?php do_links(0, 0, 1); ?><br>
			<font size="-1">change style: <?php do_styles($type);?></font>
		</td></tr>
		<tr><td bgcolor="#eeeeee" class="border">
<?php
	}
	else if ($style == 'plain')
	{
?>
<body bgcolor="#ffffff" background="images/compass.gif">
<h1>nangband: <?=$title?></h1>
<div align="right"><?php do_links(0, 0, 1); ?></div>
<hr align="right" width="50%">
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
 
		echo '<tr><td bgcolor="#ffffff" align="center">';
		echo '<font color="#555555">';
		do_links(0, 0, 1);
		echo '</font>';
		echo '<br>';
		echo '<div align="center">';
		echo '<font size="-1" color="#555555">';
		echo 'Material &copy; Nangband Developers. Contact ';
		echo '<a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>';
		echo '<br>';
		echo '<a href="http://jigsaw.w3.org/css-validator/check/referer">CSS 2.0 Compliant</a>';
		echo ' | ';
		echo '<a href="http://validator.w3.org/check/referer">HTML 4.01 Compliant</a>';
		echo '</font>';

		echo '</div></td></tr>';
		echo '</table></td></tr></table>';
	}
	else if ($style == 'plain')
	{
?>
<hr align="right" width="50%">
<div align="right">Change style: <?php do_styles($type); ?></div>
</body>
</html>
<?php
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
<!-- Page footer -->
<font color="#555555" size="-1">&copy; Nangband Developers.<br>
Contact <a href="mailto:nevermiah@hotmail.com">nevermiah@hotmail.com</a>
<br>
<a href="http://jigsaw.w3.org/css-validator/check/referer">CSS 2.0 Compliant</a> |
<a href="http://validator.w3.org/check/referer">HTML 4.01 Compliant</a>
</font>
<!-- End page footer -->
		</td>
	</tr>
</table>
<?php
	}

	echo '</body></html>';

	return;
}

// Redirect to the "real" source lister if we're not included
if (!$included[1]) header('Location: /?source=layout');

?>

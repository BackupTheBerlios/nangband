<?php
///////////////////////////////////
//                               //
// lister.php - directory lister //
//     andrew sidwell, 2002      //
//                               //
///////////////////////////////////

function show_dir_list($dirname)
{
	if ($dirname != '.') echo '<br><b>'.$dirname.'</b>';

	echo '<ul>';

	$d = dir($dirname);

	while ($entry = $d->read())
	{
		if (($entry == '.') || ($entry == '..') || ($entry == 'CVS') || ($entry == 'index.php'))
			continue;

		if (!is_dir($entry)) echo '<li>'.$entry.'</li>';
	}

	$d->close();

	echo '</ul>';
}

echo '<html><head><title>Directory lister test</title></head>';
echo '<body bgcolor="#ffffff"><font size"+1"><div align="center"><b>content/</b></div></font>';
show_dir_list('.');
echo '</body></html>';

?>

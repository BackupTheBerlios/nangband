<?php
/////////////////////////////////////////////////
//                                             //
// content/index.php - content lister for site //
//       (c) Andrew Sidwell, 2002              //
//                                             //
/////////////////////////////////////////////////

// Parse a file
function parse_file($fd)
{
	global $page_name, $page_desc, $redirect;

	while (!feof($fd))
	{
		$buffer = fgets($fd, 1024);

		if (preg_match('/Page-Name:/', $buffer))
		{
			$page_name = strstr($buffer, 'Page-Name: ');
			echo $buffer;
		}
		else if (preg_match('/Page-Desc:/', $buffer))
		{
			$page_desc = strstr($buffer, 'Page-Name: ');
		}
		if (preg_match('/Redirect:/', $buffer))
		{
			$redirect = strstr($buffer, 'Page-Name: ');
		}

	}
}

// "Open" this directory.
$dirhandle = opendir('.');

// Cycle through all the pages
while (($file = readdir($dirhandle)) != false)
{
	if (($file != 'index.php') &&
	    ($file != '.') &&
	    ($file != '..') &&
	    (!is_dir($file)))
	{
		// Find the name of the datafile
		$datafile = $file. '.';

		// Check the file exists
		if (file_exists($datafile))
		{
			$h = fopen($datafile, 'r');
			if ($h) parse_file($h);

			echo '$page_name = ' .$page_name. '<br>';
			echo '$page_desc = ' .$page_desc. '<br>';
			echo '$redirect  = ' .$redirect.  '<br><br>';

			fclose($h);
		}
	}

}

// We are done.

?>

<?php
///////////////////////////////////////
//                                   //
//  sitedata.php - site information  //
//      (c) andrew sidwell, 2002     //
//                                   //
///////////////////////////////////////

// Site name
$sitename = 'nangband';

// Filename-accessed array of "full names"
$titles['main'] = 'main page';
$titles['changes'] = 'changes';
$titles['devinfo'] = 'development';
$titles['download'] = 'downloads';
$titles['links'] = 'links';

// Number-accessed array of content filenames
$pages[0] = 'main';
$pages[1] = 'changes';
$pages[2] = 'devinfo';
$pages[3] = 'download';
$pages[4] = 'links';

// List of styles
$styles[0] = 'sidebar';
$styles[1] = 'clean';
$styles[2] = 'plain';

// Default style
$default_style = 'clean';

// Redirect to the "real" source lister if we're not included
if (!$included[1]) header('Location: /?source=files');
?>

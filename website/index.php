<?php
/*
 * index.php - the main page for the site.
 * ---------------------------------------
 *
 * started on:		07/03/01
 * by:				takkaria
 */

include('general/layout.php');

$creators = 'Andrew Sidwell';
page_header('Main Page', 'index');
?>
<p>Nangband is an Angband variant still in alpha stage, and only available via CVS.  It features a full rewrite of the savefile code, a new resists system similar to EyAngband, seperate stat bonuses, object recall and three new races from Antiband.</p>

<p><em>Berlios.de</em> hosts the project, including CVS, bug reporting facilities, website hosting, file releases and mailing lists.  Many thanks to them for hosting it.</p>

<p>There are two mailing lists for Nangband; click on one to be taken to it's subscription/unsubscription page.
<ul>
<li><a href="https://lists.berlios.de/mailman/listinfo/nangband-discussion">nangband-discussion</a>: General discussion about Nangband.</li>
<li><a href="https://lists.berlios.de/mailman/listinfo/nangband-developer">nangband-developer</a>: Nangband developer's list.</li>
</ul>
</p>

<p>The CVS can be checked out using the following command:<br>
<pre>cvs -z3 -:d:pserver:anonymous@cvs.nangband.berlios.de:/cvsroot/nangband co nangband</pre>
Replace 'co nangband' with 'co nang-web' if you want to check out this website instead.</p>
<?
page_footer($creators);
?>

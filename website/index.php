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
<p>Nangband is a farirly new Angband variant, based on the plain version of Angband's (Vanilla) CVS.
Nangband is normally updated to incorporate any useful changes made in the Vanilla tree within a few
days.</p>
<p>There is no release as of yet, because it is very much in development. However, you may download via CVS,
or you can wait for a near-stable prerelease which should happen in the next week or two. The estimated
release date of Nangband 0.1.0 is the first week of April.</p>
<p><font size="+1">The CVS entered unplayable (not unstable) state at 
21:03 GMT time, 2002-03-08 (ceyr-mm-dd).</font></p>
<p><em>Berlios.de</em> is hosting the project, including the CVS, the website, file releases and the mailing
lists. If you are interested in this project, please join 'nangband-discussion'. A direct link to the
mailing page is: <a href="http://developer.berlios.de/mail/?group_id=266">http://developer.berlios.de/mail/?group_id=266</a>. 
The CVS can be checked out using the following command:<br>
<pre>
cvs -z3 -:d:pserver:anonymous@cvs.nangband.berlios.de:/cvsroot/nangband co nangband
</pre><br>
Replace 'co nangband' with 'co nang-web' if you want to check out this website instead.</p>
</p>
<?
page_footer($creators);
?>

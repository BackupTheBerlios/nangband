<?php
//
// Function: rtime - human-readable time.
//
// Description:
//    Implements the rtime function, which presents a readable time, of the form:
//    "5 years, 4 days, 2 hours, 44 minutes, and 25 seconds", such as the output
//    given my readableTime() in Perl.
//
// Usage:
//    $text = rtime([int timestamp [, int max_level])
//
//    If "timestamp" is omitted, then the current epoch time will be used, which
//    will output the time since January 1st, 1970, at midnight.
//
//    "max_level" specifies the smallest unit of time to go to; passing 5 here will
//    go down to seconds, 4 will go down to minutes, 3 will go to hours, 2 will go to
//    days, and 1 will only go to years.  This does not perform rounding, so values
//    will not be very accurate.
//
// This function may be used freely in any projects, as long as it is credited to myself
// (Andrew Sidwell); I would appriciate it, that if you make any changes to this, you
// send them to me at nevermiah at hotmail dot com.  Thanks.
//
function rtime($time = time(), $max_level = 5)
{
	// Note we require gmtime()
	if (!function_exists(gmtime)) return;

	// Grab the times
	$ts = gmtime($time, 1);

	// The date starts at a weird year
	$ts[tm_year] -= 70;

	// Start off with years.
	if ($ts[tm_year])
		$result .= $ts[tm_year]. ' year' .(($ts[tm_year] > 1) ? 's' : '');

        // Now days.
	if ($max_level == 1) return ($result);
	if ($ts[tm_yday] && !$ts[tm_hour] && !$ts[tm_min] && !$ts[tm_sec]) $result .= ' and ';
	else if ($ts[tm_year]) $result .= ', ';
	if ($ts[tm_yday]) $result .= $ts[tm_yday]. ' day' .(($ts[tm_yday] > 1) ? 's' : '');

	// Do hours.
	if ($max_level == 2) return ($result);
	if ($ts[tm_hour] && !$ts[tm_min] && !$ts[tm_sec]) $result .= ' and ';
	else if ($ts[tm_year] || $ts[tm_yday]) $result .= ', ';
	if ($ts[tm_hour]) $result .= $ts[tm_hour]. ' hour' .(($ts[tm_hour] > 1) ? 's' : '');

        // Now minutes.
	if ($max_level == 3) return ($result);
        if ($ts[tm_min] && !$ts[tm_sec]) $result .= ' and ';
        else if ($ts[tm_year] || $ts[tm_yday] || $ts[tm_hour]) $result .= ', ';
        if ($ts[tm_min]) $result .= $ts[tm_min]. ' minute' .(($ts[tm_min] > 1) ? 's' : '');

	// Add the seconds on, if at all
	if ($max_level == 4) return ($result);
	if ($ts[tm_sec] && $result) $result .= ' and ';
	if ($ts[tm_sec]) $result .= $ts[tm_sec]. ' second' .(($ts[tm_sec] > 1) ? 's' : '');

	// Return the formatted time
	return ($result);
}
?>

<?php
//
// Description:
//    Implements the gmttime function if missing from the PHP distribution
//
//
// Function: gmttime
//
// Description:
//   Returns an array indexed as by the localtime() function:
//   - 0 or tm_sec: Seconds
//   - 1 or tm_min: Minutes
//   - 2 or tm_hour: Hour
//   - 3 or tm_mday: Day of the month
//   - 4 or tm_mon: Month of the year
//   - 5 or tm_year: Years since 1900
//   - 6 or tm_wday: Day of the week
//   - 7 or tm_yday: Day of the year
//   - 8 or tm_isdst: Is daylight saving time in effect
//   - tm_fyear: Full year (only available with associative array)
//
// Arguments:
//   - Timestamp
//   - Boolean (for associative indexing: 0 = off, 1 = on)
//
// Returns:
//   An array on success,
//   false on failure.
//
function gmtime($dTimestamp = '', $bAssoc = 0) {
	// Evaluate how much difference there is between local and GTM/UTC
	// Don't forget to correct for daylight saving time...
	$aNow = localtime();
	$iDelta = gmmktime(1, 1, 1, 1, 1, 1970, $aNow[8]) - mktime(1, 1, 1, 1, 1,
1970, $aNow[8]);

	if (!$bAssoc) {
		if ($dTimestamp == '') {
			return localtime(time() - $iDelta, 0);
		} else {
			return localtime($dTimestamp - $iDelta, 0);
		}
	} else {
		// For associative array, add full year index
		if ($dTimestamp == '') {
			$aGMTTime = localtime(time() - $iDelta, 1);
		} else {
			$aGMTTime = localtime($dTimestamp - $iDelta, 1);
		}
		$aGMTTime['tm_fyear'] = $aGMTTime['tm_year'] + 1900;
		return $aGMTTime;
	} // End [IF] return associative array?
} // End [FUNCTION] gmttime
?>

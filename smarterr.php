#!/usr/bin/env php
<?php
// smarterr smartctl S.M.A.R.T. error log parser
// © John Peterson. License GNU GPL 3.

$argv[1];
$out = $argv[2];
$usage = 'Usage: smarterr.php [command | infile] outfile';

// Exceptions
if (!isset($argv[2])) { echo $usage.PHP_EOL; return; }
if (!strncmp($argv[1], 'smartctl', 8)) {
	if (!preg_match('/^smartctl.*(xall|error|xerror).*/', $argv[1])) {
		echo 'Unsupported command: ' . $argv[1] . PHP_EOL; return;
	}
} else if (!file_exists($argv[1])) {
	echo $argv[1] . ' not found' . PHP_EOL; return;
} else {
	$in = $argv[1];
}

// Parse input
function parse_err($lines) {
	$log_arr = array();
	$first_log = false;
	foreach ($lines as $i => $line) {
		if (strpos($line, 'occurred at disk power-on lifetime')) {
			if ($first_log) $log_arr[$num] = $log;
			$num = explode(' ', $line)[1];
			$first_log = true;
			$log = "";
		}
		else
			if ($first_log && preg_match('[^\w]', $line)) goto ret;
		if ($first_log) $log .= $line;		
	}
	ret:
	if (strlen($log)) $log_arr[$num] = $log;
	return $log_arr;
}

// Write file
function write_file($data, $filename) {
	$f = fopen($filename, 'w');
	foreach ($data as $i => $line)
		fwrite($f, $line);
	fclose($f);
}

// Read S.M.A.R.T. log
function passthru2($cmd) {
	$descriptorspec = array(array("pipe", "r"), array("pipe", "w"), array("pipe", "w"));
	$process = proc_open($cmd, $descriptorspec, $pipes);
	if (is_resource($process)) {		
		$ret = stream_get_contents($pipes[1]);
		fclose($pipes[0]);
		fclose($pipes[1]);
		fclose($pipes[2]);
		proc_close($process);
	}
	return $ret;
}

// Read outfile
@$log = parse_err(file($out));
// Append new errors
if (isset($in))
	$log = array_diff_key(parse_err(file($in)), $log) + $log;
else {
	$xr = passthru2($argv[1]);
	$xr = preg_replace("/\R/", "\n\0", $xr);
	$xr = explode("\0", $xr);
	$log = array_diff_key(parse_err($xr), $log) + $log;
}
krsort($log);
write_file($log, $out);
?>
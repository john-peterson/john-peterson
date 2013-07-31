#!/usr/bin/env php
<?php
// change Eclipse project path
// © John Peterson. License GNU GPL 3.

// use
$usage = "Usage: ${argv[0]} [-i] from to

Replace the path component \"from\" with \"to\"

-i	case insensitive";

// argument
array_shift($argv);
$arg = array_shift($argv);

if ($arg == "-i") {
	$insensitive = true;
	$from = array_shift($argv);
}
else
	$from = $arg;
$to = array_shift($argv);

// exception
if (!isset($to)) {
	echo $usage;
	return;
}

// find files
$ite = new RecursiveDirectoryIterator(".");
foreach (new RecursiveIteratorIterator($ite) as $file) {
	$name = basename($file);
	if ($name == ".location") {
		$ret = replace($file, $from, $to, $insensitive);
		if (is_array($ret)) {
			list($before, $after) = $ret;
			echo substr($file, 2) . "\n$before\n$after\n\n";
		}
	}
}

// replace in path
function replace($file, $from, $to, $insensitive) {
	// read
	$data = file_get_contents($file);

	// path length
	$loc = strpos($data, "URI//file:");
	$len = ord(substr($data, $loc - 1, 1));

	// file components
	$path = substr($data, $loc, $len);
	$before = substr($data, 0, $loc - 2);
	$after = substr($data, $loc + strlen($path));
	
	if ($insensitive)
		$exist = stripos($path, $from);
	else
		$exist = strpos($path, $from);

	if ($exist === false)
		return false;

	// before path
	$ret[] = $path;

	// replace in path
	if ($insensitive)
		$path = str_ireplace($from, $to, $path);
	else
		$path = str_replace($from, $to, $path);
		
	// after path
	$ret[] = $path;

	// concatenate file
	$data = $before . pack('n', strlen($path)) . $path . $after;

	// write
	file_put_contents($file, $data);

	return $ret;
}
?>
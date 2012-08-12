#!/usr/bin/env php
<?php
// diffdiff diff comparison utility.
// © John Peterson, GNU GPL 3

$f1 = $argv[1];
$f2 = $argv[2];
$usage = 'Usage: diffdiff.php diff1 diff2';

// Exceptions
if (!isset($f1) || !isset($f2)) { echo $usage . PHP_EOL; return; }

// Parse file list
function files_list($lines) {
	foreach ($lines as $line_num => $line) {
		if (!strncmp($line, 'diff --git', 10)) {
			$path_parts = pathinfo($line);
			$filename[] = $path_parts['filename'].(strlen($path_parts['extension'])?('.'.$path_parts['extension']):'');
		}
	}
	return $filename;
}
// Strip unwanted rows
function file_array($lines, $filenames) {
	foreach ($lines as $line_num => $line) {
		if (!strncmp($line, 'diff --git', 10)) {
			$path_parts = pathinfo($line);
			$filename = $path_parts['filename'].(strlen($path_parts['extension'])?('.'.$path_parts['extension']):'');
			if (array_search($filename, $filenames)) $include = true; else $include = false;
		}
		// Diff lines
		if (strncmp($line, 'diff --git', 10)
		&& strncmp($line, '+', 1)
		&& strncmp($line, '-', 1)) continue;
		if ($include) $line_out[] = $line;
	}
	return $line_out;
}

// Write file
function write_array($file, $filename) {
	$f = fopen($filename, 'w');
	foreach ($file as $line_num => $line)
		fwrite($f, $line);
	fclose($f);
}

$files1 = files_list(file($f1));
$files2 = files_list(file($f2));

$combined_filenames = array_intersect($files1, $files2);
// Show unique files
echo 'Unique in second file:'.PHP_EOL;
print_r(array_diff($files2, $files1));

$file1 = file_array(file($f1), $combined_filenames);
$file2 = file_array(file($f2), $combined_filenames);

write_array($file1, '$~file1.tmp');
write_array($file2, '$~file2.tmp');

echo 'Diff in common files:'.PHP_EOL;
echo passthru('diff -u $~file1.tmp $~file2.tmp');

unlink('$~file1.tmp');
unlink('$~file2.tmp');
?>
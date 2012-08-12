#!/usr/bin/php
wplsync WPL (Windows Media Player Playlist) sync.
(C) John Peterson. License GNU GPL 3.
<?php
$from = $argv[1];
$to = $argv[2];
$usage = 'Usage: smarterr.php inpath outpath';

// Exceptions
if (!isset($argv[2])) { echo $usage.PHP_EOL; return; }
if (!is_dir($argv[1])) { echo $argv[1]." is not a directory".PHP_EOL; return; }

/**
 * XML decode.
 */
function xml_entity_decode($str) {
	$str = str_replace("&apos;", "'", $str);
	return html_entity_decode($str);
}

/**
 * Append tail slash.
 */
function append_slash($path) {
	if ($path[strlen($path)-1] != '\\') $path.='\\';
	return $path;
}

/**
 * Return dirname (with UNC path support).
 */
function pathinfo_dirname($path) {
	if (is_dir($path)) $path = append_slash($path);
	$path = substr($path, 0, strrpos($path,'\\'));
	return $path;
}

/**
 * Return dir excluding drive.
 */
function pathinfo_dirnext($path, $needle = '\\') {
	return substr($path, strpos($path, $needle)+strlen($needle));
}

/**
 * Makes directory, returns TRUE if exists or made.
 *
 * @param string $pathname The directory path.
 * @return boolean returns TRUE if exists or made or FALSE on failure.
 */
function mkdir_recursive($pathname, $mode = 0777) {
	// echo "Create\t".$pathname.PHP_EOL;
    is_dir(dirname($pathname)) || mkdir_recursive(dirname($pathname), $mode);
    return is_dir($pathname) || @mkdir($pathname, $mode);
}

/**
 * Is music file name.
 */
function is_music_file($s) {
	$e = pathinfo($s, PATHINFO_EXTENSION);
	$x = array("ogg","mp3","wma");
	if (array_search($e, $x) !== false) return true;
	return false;
}

/**
 * Copy file, create target path if necessary, don't overwrite.
 */
function copy2($from, $to) {
	echo "Copy\t$from\n\t-> $to\n";
	if (file_exists($to)) return;
	if (!mkdir_recursive(pathinfo_dirname($to))) echo "Failed to create\t".dirname($to);
	if (!@copy($from, $to)) {
		echo "Failed to copy\t$from\n\t\t-> $to";
		exit();
	}
}

/**
 * Remove extra files.
 *
 * @param array $list The correct list of files.
 * @param string $target The target directory path.
 */
function unlink_extra($list, $target) {
	$existing = dir_recursive($target);
	$extra = array_diff($existing, $list);
	foreach ($extra as $i => $line) {
		echo "Removing\t$line\n";
		unlink($line);
	}
}

/**
 * Create list of files in directory.
 *
 * @param array $path The directory path.
 * @param array $list The file list.
 */
function dir_recursive($path, $list = array()) {
	if ($handle = opendir($path)) {
		while (false !== ($entry = readdir($handle))) {
			if (is_dir(append_slash($path).$entry)) {
				if ($entry != "." && $entry != "..")
					$list = dir_recursive(append_slash($path).$entry, $list);
			} else {
				$list[] = append_slash($path).$entry;
				if (!(count($list)%25)) echo "Listing...\t".count($list).PHP_EOL;
			}
		}
		closedir($handle);
	}
	return $list;
}

/**
 * Copy list of files.
 *
 * @param string array $from The source directory paths.
 * @param string array $to The target directory paths.
 */
function copy_list($from, $to) {
	for($i = 0; $i < count($from); $i++)	
		copy2($from[$i], $to[$i]);
	echo "\nCopied ".count($from)." files\n";
}

// collect wpl files
function list_wpl($from, $to) {
	$lfrom = array(); $lto = array();
	if ($handle = opendir($from)) {
		while (false !== ($entry = readdir($handle))) {			
			if (is_dir(append_slash($path).$entry)) {
				// if ($entry != "." && $entry != "..") {
				// }
			} else {
				list($_lfrom, $_lto) = parse_wpl(file(append_slash($from).$entry), $from, $to);
				$lfrom = array_merge($_lfrom, $lfrom);
				$lto = array_merge($_lto, $lto);
			}
		}
		closedir($handle);
	}
	return array($lfrom, $lto);
}

// parse wpl
function parse_wpl($lines, $from, $to) {
	$lfrom = array(); $lto = array();
	foreach ($lines as $i => $line) {
		if (strpos($line, '<media')) {
			$elem = explode('"',$line);
			$path_from = xml_entity_decode(utf8_decode($elem[1]));
			if (!is_music_file($path_from)) continue;
			// relative path
			if (!strncmp($path_from, '..\\', 3) || (!strncmp($path_from, '\\', 1) && strncmp($path_from, '\\\\', 2))) {
				$lfrom[] = append_slash(pathinfo_dirname($from)).$path_from;
				$lto[] = append_slash(pathinfo_dirname($to)).pathinfo_dirnext($path_from);
			// absolute path: assume source paths is called 'Music' and use source path after '...Music\' as target path
			} else {
				$lfrom[] = $path_from;
				$lto[] = append_slash(pathinfo_dirname($to)).pathinfo_dirnext($path_from, 'Music\\');
			}
		}
	}
	return array($lfrom, $lto);
}

// file list
list($lfrom, $lto) = list_wpl($from, $to);
// copy files
copy_list($lfrom, $lto);
// remove extra files
echo PHP_EOL."Removing extra files in target directory...\t".PHP_EOL;
unlink_extra($lto, $to);
?>
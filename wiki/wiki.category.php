#!php
<?php
// Category reader.
// (C) John Peterson. License GNU GPL 3.
include('../common/common.php');
$usage = "Usage: ${argv[0]} category [server].";

// exceptions
if (!isset($argv[1])) { echo $usage.PHP_EOL; return; } else $c = $argv[1];
if (!isset($argv[2])) $s = "localhost"; else $s = $argv[2];
if (isset($argv[3])) $i = explode(";", $argv[3]);
$f = $c; file_put_contents("$f.raw.log", "");
$A = array();
$duplicate = false;
$STDERR = fopen('php://stderr', 'w+');

function file_get_contents_header($url) {
	$opts = array('http'=>array('method'=>"GET", 'header'=>"User-Agent: PHP ".PHP_VERSION));
	$context = stream_context_create($opts);
	return file_get_contents($url, false, $context);
}

function array_search_word($s, $a) {
	foreach ($a as $b) {
		if (preg_match("/\b$b\b/i", $s)) return true;
	}
	return false;
}
function array_search_string($s, $a) {
	foreach ($a as $b) {
		if (stripos($s, $b) !== false) return true;
	}
	return false;
}
function array_search_whole_string($s, $a) {
	foreach ($a as $b) {
		if (strtolower($s) == strtolower($b)) return true;
	}
	return false;
}

function strip_wiki($s) {
	$s = preg_replace('/\r/', '', $s);
	$s = preg_replace('/\'{2,3}(.*?)\'{2,3}/', '$1', $s);
	$s = preg_replace("/\*\*(.*?)\*\*/", '$1', $s);
	$s = preg_replace("/__(.*?)__/", '$1', $s);
	$s = preg_replace("/\/\/(.*?)\/\//", '$1', $s);
	$s = preg_replace('/\-\-(.*?)\-\-/', '$1', $s);
	$s = preg_replace('/=+.*=+/', '', $s);
	$s = preg_replace('/\[\[[\w-]*:.*\]\]/', '', $s);
	$s = preg_replace('/\[\[[^|\]]*?\|([^|\]]*?)\]\]/', '$1', $s);
	$s = preg_replace('/\[\[(.*?)\]\]/', '$1', $s);
	$s = preg_replace('/\[.*? (.*?)\]/', '$1', $s);
	$s = preg_replace('/{{.*?}}/s', '', $s);
	$s = preg_replace('/\n?{{.*\n}}/s', '', $s);
	$s = preg_replace('/{\|.*?\|}/s', '', $s);
	$s = preg_replace('/<ref.*?\/(ref)?>/', '', $s);
	$s = preg_replace('/<\/?.*?>/s', '', $s);
	$s = preg_replace('/\n[:*#].*/', '', $s);
	return $s;
}

function get_category($c, $ct = "") {
	global $STDERR, $f, $s, $i, $A, $n;
	if (!empty($ct)) $ct .= " → "; $ct .= $c;	
	$skip_word = array('portal','list','lists');
	$skip_string = array('portal:','template:','talk:');	
	fwrite($STDERR, sprintf("\r\033[K%d Reading category:%s ...", $n, $c));
	$url = "http://$s/w/api.php?action=query&list=categorymembers&cmlimit=max&format=xml&cmtitle=category:".urlencode($c);
	$xml = file_get_contents_header($url);
	if (count(xpath($xml, "//categorymembers/cm/@title")) == 0) {
		fwrite($STDERR, sprintf("category:%s not found on %s ...\n", $c, $s));
		return;
	}
	$a = xpath($xml, "//categorymembers/cm/@title");
	// print_r($a);
	foreach ($a as $b) {
		if (array_search_word($b, $skip_word) || array_search_string($b, $skip_string)) continue;
		if (stripos($b, 'category:') !== false) {
			$c = substr($b,strpos($b,':')+1);
			if (array_search_whole_string($c, $i)) continue;
			get_category($c, $ct);
		}
		else {
			if ($duplicate) {
				if (array_search_whole_string($b, $A)) continue;
				$A[] = $b;
			}
			$n++;
			fwrite($STDERR, sprintf("\r\033[K%d %s → %s ...", $n, $ct, $b));
			$t = file_get_contents_header("http://$s/w/index.php?action=raw&title=".urlencode($b));
			file_put_contents("$f.raw.log", "¶$n $ct → $b\n".$t."\n", FILE_APPEND);
			echo strip_wiki($t);
		}
	}
}

function count_lines($f) {
	$c = 0;
	$h = fopen($f, "r");
	if (!$h) return 0;
	while (!feof($h)) {
		$line = fgets($h);
		$c++;
	}
	fclose($h);
	return $c;
}

// single article parser
// $s = file_get_contents_header("article.test.txt"); echo strip_wiki($s);
// multi-article parser
// $f = "$f.raw.log";
// $handle = @fopen($f, "r");
// $i = 0;
// $c = count_lines($f);
// if ($handle) {
	// $STDERR = fopen('php://stderr', 'w+');
	// while (($s = fgets($handle)) !== false) {
		// if ($i%10000 == 0) fwrite($STDERR, sprintf("\r\033[KReading %d of %d ...", $i, $c));
		// $i++;
		// if (!strncmp($s, "¶", 1))
			// $a .= $s;
			// continue;
		// } else {
			// echo strip_wiki($a);
			// $a = "";
		// }		
	// }
	// if (!feof($handle)) echo "Error: unexpected fgets() fail\n";
	// fclose($handle);
// }
// get category
get_category($c);
?>
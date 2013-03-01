<?php
// Shared code.
// © John Peterson. License GNU GPL 3.

define('SEP', ';');

// xml
function xpath_request($s, $q, $html = false) {
	libxml_use_internal_errors(true);
	$doc = new DOMDocument;
	if (!$html)
		$doc->loadXML($s);
	else
		$doc->loadHTML($s);	
	$x = new DOMXPath($doc);
	if ($q[0] == '/' || $q[0] == '*')
		return $x->query($q);
	else
		return $x->evaluate($q);
}
function xpath($s, $q, $html = false) {
	$result = xpath_request($s, $q, $html);
	if (!is_object($result)) return $result;
	foreach($result as $node)
		$x[] = $node->nodeValue;
	if (count($x) == 1) return $x[0]; else return $x;
}
function xpath2array($n) {
	foreach($n as $o) {
		$d = new DOMDocument;
		$i = $d->importNode($o,true);
		$d->appendChild($i);
		$a = xml2array($d->saveXML())['attributes'];
		$r[$a['name']] = $a['value'];
	}
	return $r;
}
function xpath_multi($s, $q, $html = false) {
	$result = xpath_request($s, $q, $html);
	return xpath2array($result);
}
function xml2array_($xml) {
	foreach ($xml->children() as $n) {
		$n_a = (array)$n;
		$i = array();
		$i['name'] = $n->getName();
		$i['value'] = trim(strval($n));
		$i['attributes'] = $n_a['@attributes'];
		if(count($n->children()) > 0) {
			$i['children'] = xml2array_($n);
		}
		$a[] = $i;
	}
	return $a;
}
function xml2array($xml) {
	if (!is_object($xml))
		$xml = new SimpleXMLElement($xml);
	$a['name'] = $xml->getName();
	$xml_a = (array)$xml; $a['attributes'] = $xml_a['@attributes'];
	if (is_object($xml->children()))
		$a['children'] = xml2array_($xml);
	return $a;
}

// network
function curl_request($url, &$cookie='', $referer='', $post=array(), $agent = 'PHP') {
	global $debug;

	static $i = 0; $i++;
	if ($debug) echo "Saving to $i: $url\n";

	$is_post = !empty($post);
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $url);
	if (!empty($cookie)) curl_setopt($ch, CURLOPT_COOKIE, get_cookies($cookie));
	if (!empty($referer)) curl_setopt($ch, CURLOPT_REFERER, $referer);	
	curl_setopt($ch, CURLOPT_USERAGENT, $agent);
	if ($is_post) curl_setopt($ch, CURLOPT_POST, true);
	if ($is_post) curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($post));
	curl_setopt($ch, CURLOPT_HEADER, true);
	curl_setopt($ch, CURLINFO_HEADER_OUT, true);	
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
	$result = curl_exec($ch);
	$code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	$h_out = curl_getinfo($ch, CURLINFO_HEADER_OUT);
	list($header, $content) = parse_return($result);
	// echo PHP_EOL.$h_out.$header.PHP_EOL.http_build_query($post).PHP_EOL;
	// echo $content.PHP_EOL.PHP_EOL;
	update_cookies($header, $cookie);
	if ($debug) file_put_contents(create_filename('saved', $i, 'html'), $content);
	curl_close($ch);
	flushi();
	if ($code == 200) return $content; else return array(get_httpcode($header));
}
function get_cookies($c) {
	if (empty($c)) return;
	foreach ($c as $k=>$d) {
		$e .= $k.'='.$d.'; ';
	}
	return substr($e, 0, -2);
}
function update_cookies($h, &$cookie) {
	$c = get_setcookie($h);
	if (empty($c)) return;
	if (count($c) < 2) $c = array($c);
	foreach ($c as $d) {
		$k = explode('=', $d, 2);
		$v = explode(';', $k[1], 2);		
		$cookie[$k[0]] = $v[0];
	}
}
function ping($host, $timeout = 1) {
	global $argv;
	$package = "\x08\x00\x7d\x4b\x00\x00\x00\x00PingHost";
	$socket = socket_create(AF_INET, SOCK_RAW, 1);
	if (!$socket) {
		printf("Unable to ping. Run as 'sudo %s'.\n", implode(' ',$argv));
		return "";
	}
	socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, array('sec'=>$timeout,'usec'=>0));
	socket_connect($socket, $host, null);
	$ts = microtime(true);
	socket_send($socket, $package, strlen($package), 0);
	if (socket_read($socket, 0xff))
		$result = round(((microtime(true)-$ts)*1000), 0);
	else
		$result = "";

	socket_close($socket);
	return $result;
}
function distance($l1, $l2) {
	list($la1,$lo1) = $l1;
	list($la2,$lo2) = $l2;
	return (6371*pi()*sqrt(($la2-$la1)*($la2-$la1)+cos(deg2rad($la2))*cos(deg2rad($la1))*($lo2-$lo1)*($lo2-$lo1))/180);
}
function get_address() {
	return file_get_contents("http://automation.whatismyip.com/n09230945.asp");
}
function a2loc($address) {
	$h = file_get_contents("http://www.geoiptool.com/en/?IP=".$address);
	$la = xpath($h, "//span[contains(text(),'Latitude')]/../../td/text()", true);
	$lo = xpath($h, "//span[contains(text(),'Longitude')]/../../td/text()", true);
	return array($la,$lo);
}
function ison2eng($s) {
	$s = str_replace('.', '', $s);
	$s = str_replace(',', '.', $s);
	return $s;
}
function parse_return($s) {
	$s = explode("\r\n\r\n", $s, 2); 
	$header = isset($s[0]) ? $s[0] : '';
	$content = isset($s[1]) ? trim($s[1]) : '';
	return array($header, $content);
}
function http_header() {
	foreach ($_SERVER as $h=>$v)
		if (preg_match('/HTTP_(.+)/', $h)) $s .= "$h = $v".PHP_EOL;
	return $s;
}
function get_header_element($s, $k) {
	foreach(preg_split("/(\r?\n)/", $s) as $l) {
		$v = explode(':', $l, 2);
		$x[$v[0]][] = trim($v[1]);
	}
	if (count($x[$k]) == 1) $x[$k] = $x[$k][0];
	return $x[$k];
}
function get_contenttype($s) {
	return get_header_element($s, 'Content-Type');
}
function get_cookie($s) {
	return get_header_element($s, 'Cookie');
}
function get_setcookie($s) {
	return get_header_element($s, 'Set-Cookie');
}
function get_host($s) {
	return get_header_element($s, 'Host');
}
function get_referer($s) {
	return get_header_element($s, 'Referer');
}
function get_useragent($s) {
	return get_header_element($s, 'User-Agent');
}
function get_httpcode($s) {
	foreach(preg_split("/(\r?\n)/", $s) as $l)
		return $l;
}

// ini
function ini_write($cat, $f) {
	ksort($cat, SORT_NATURAL|SORT_FLAG_CASE);
	foreach($cat as $key=>&$v) {
		if (empty($key)) continue;
		$out .= $key.' = ';
		sort($v);
		foreach($v as $x) {
			$out .= $x.SEP;
		}
		$out = substr($out, 0, -1);
		$out .= "\n";
	}
	file_put_contents($f, $out);
}
function ini_read($f) {
	$ini = file($f);
	foreach($ini as $line) {
		$line = trim($line);	
		if (!strncmp($line, SEP, 1)) continue;
		$key = trim(strstr($line, '=', true));
		$v = trim(substr($line, strpos($line, '=')+1));
		$v = explode(SEP, $v);
		foreach ($v as $x) {
			$out[$key][] = trim($x);
		}
	}
	return $out;
}

// walk

// functions that accept walk
function trim_ex(&$v, $k) {
	$v = trim($v);
}

// walk wrappers
function trim_w($v) {
	if (is_array($v)) array_walk_recursive($v, 'trim_ex');
	return $v;
}

// other
function flushi() {
	flush();
	ob_flush();
}
function url_exists($s) {
	@file_get_contents($s,0,NULL,0,1);
	$type = get_contenttype(arr2text($http_response_header));
	if ($type == 'text/csv' || $type == 'application/zip') return true; else return false;
}
function file_existsi($f) {
	if (is_url($f)) return url_exists($f); else return file_exists($f);
}
function is_cli() {
	return !isset($_SERVER['HTTP_HOST']);
}
function is_url($s) {
	return filter_var($s, FILTER_VALIDATE_URL);
}
function is_utf8($s) {
	return mb_detect_encoding($s, 'UTF-8', true);
}
function utf8_encodei($s) {
	if (is_array($s)) {
		foreach($s as &$v) {			
			if (!is_utf8($s)) $v = utf8_encode($v);
		}
		return $s;
	} else
		if (is_utf8($s)) return $s; else return utf8_encode($s);
}
function create_filename($p, $f, $e) {
	$bad = array_merge(
        array_map('chr', range(0,31)),
        array("<", ">", ":", '"', "/", "\\", "|", "?", "*", ";"));
	$f = str_replace($bad, "", $f);
	return $p.'/'.substr($f, 0, 257-strlen(getcwd())-strlen($p)-1-strlen($f)-1-strlen($e)).'.'.$e;
}
function rand_hex() {
	$min = hexdec("100");
	$max = hexdec("ffff");
	$rand = mt_rand($min, $max);
	$hex = dechex($rand);
	// if (strlen($hex) != 4) $hex = rand_hex();
	return $hex;
}
function arr2text($a) {
	foreach($a as $v) $s .= $v.= "\r\n";
	return substr($s, 0, -2);
}
function text2arr($s) {
	foreach(preg_split("/(\r?\n)/", $s) as $l) $v[] = trim($l);
	return $v;
}
function trim_first_line($s) {
	$s = text2arr($s);
	array_shift($s);
	return arr2text($s);
}
?>
<?php
// mediawiki API interface
// (C) John Peterson, GNU GPL 3
class mw_cm {
static function curl_request($url, &$cookie='', $referer='', $post=array()) {
	$is_post = !empty($post);
	$useragent = "PHP";	
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $url);
	if (!empty($cookie)) curl_setopt($ch, CURLOPT_COOKIE, self::get_cookies($cookie));
	if (!empty($referer)) curl_setopt($ch, CURLOPT_REFERER, $referer);	
	curl_setopt($ch, CURLOPT_USERAGENT, $useragent);
	if ($is_post) curl_setopt($ch, CURLOPT_POST, true);
	if ($is_post) curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($post));
	curl_setopt($ch, CURLOPT_HEADER, true);
	curl_setopt($ch, CURLINFO_HEADER_OUT, true);	
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
	$result = curl_exec($ch);
	$code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	$h_out = curl_getinfo($ch, CURLINFO_HEADER_OUT);
	list($header, $content) = self::parse_return($result);
	// echo PHP_EOL.$h_out.$header.PHP_EOL.http_build_query($post).PHP_EOL;
	// echo $content.PHP_EOL.PHP_EOL;
	self::update_cookies($header, $cookie);
	curl_close($ch);
	self::flushi();
	if ($code == 200) return $content; else return array(get_httpcode($header));
}

static function parse_return($s) {
	$s = explode("\r\n\r\n", $s, 2); 
	$header = isset($s[0]) ? $s[0] : '';
	$content = isset($s[1]) ? trim($s[1]) : '';
	return array($header, $content);
}

static function flushi() {
	flush();
	ob_flush();
}

static function xml2arr($xml) {
	$p = xml_parser_create();
	xml_parse_into_struct($p, $xml, $xml);
	xml_parser_free($p);
	return $xml;
}

static function get_cookies($c) {
	if (empty($c)) return;
	foreach ($c as $k=>$d) {
		$e .= $k.'='.$d.'; ';
	}
	return substr($e, 0, -2);
}

static function update_cookies($h, &$cookie) {
	$c = self::get_setcookie($h);
	if (empty($c)) return;
	if (count($c) < 2) $c = array($c);
	foreach ($c as $d) {
		$k = explode('=', $d, 2);
		$v = explode(';', $k[1], 2);		
		$cookie[$k[0]] = $v[0];
	}
}

static function get_header_element($s, $k) {
	foreach(preg_split("/(\r?\n)/", $s) as $l) {
		$v = explode(':', $l, 2);
		$x[$v[0]][] = trim($v[1]);
	}
	if (count($x[$k]) == 1) $x[$k] = $x[$k][0];
	return $x[$k];
}
static function get_contenttype($s) {
	return get_header_element($s, 'Content-Type');
}
static function get_cookie($s) {
	return get_header_element($s, 'Cookie');
}
static function get_setcookie($s) {
	return self::get_header_element($s, 'Set-Cookie');
}
static function get_host($s) {
	return get_header_element($s, 'Host');
}
static function get_referer($s) {
	return get_header_element($s, 'Referer');
}
static function get_useragent($s) {
	return get_header_element($s, 'User-Agent');
}
static function get_httpcode($s) {
	foreach(preg_split("/(\r?\n)/", $s) as $l)
		return $l;
}
}
?>
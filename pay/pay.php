<?php
// Payment history downloader
// © John Peterson. License GNU GPL 3.
include_once('../common/common.php');

// settings
$debug = true;

function get_post_login($html, $userid, $pin) {
	$post = xpath_multi($html, "//form//input", true);
	if (!sizeof($post)) {
		echo "Post fields not found\n";
		exit;
	}
	$post['JAVASCRIPT_DETECTED'] = 'true';
	$post['userid'] = $userid;
	$post['pin'] = $pin;
	return $post;
}
function get_post_transaction($html, $i) {
	$post = xpath_multi($html, "//form[contains(@name,'accountTransactions')]//input|//select", true);
	if (!sizeof($post)) {
		echo "Post fields not found\n";
		exit;
	}
	$post['defaultcommand'] = 'accounttransactions$getnewaccounttransactions';
	$post['transactionaccount'] = '0';
	$post['transactionPeriod'] = $i;
	return $post;
}

function get_url_base() {
	return "https://internetbanken.privat.nordea.se/nsp/";
}
function get_url_baselogin() {
	return get_url_base().'login';
}
function get_url_core() {
	return get_url_base().'core';
}
function get_url_login($html) {
	$url = xpath($html, "//a[contains(@href,'commonlogintab=2')]/@href", true);
	return get_url_base().$url;
}
function get_url_account($html) {
	$url = xpath($html, "//a[contains(@href,'accountsoverview')]/@href", true);
	return get_url_base().$url;
}
function get_url_csv($html) {
	$url = xpath($html, "//a[contains(@href,'output_type=csv')]/@href", true);
	return get_url_base().$url;
}

function request($url, &$cookie, $referer='', $post='') {
	$html = curl_request($url, $cookie, $referer, $post, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:11.0) Gecko/20100101 Firefox/11.0");
	if (!is_array($html))
		return $html;
	else {
		echo "Server returned {$html[0]}. Are the credentials correct?";
		exit;
	}
}

function message_login() {
	echo "Logging in…\n";
}

function message_end() {
	echo "36 months retrieved\n";
}

function login($userid, $pin) {
	global $debug;

	message_login();
	// get login url
	$url = get_url_baselogin();
	if ($debug)
		echo "Requesting session from: $url\n";
	$html = request($url, $cookie);
	if ($debug)
		echo "cookie:\n" . print_r($cookie, true) . "\n";
	$referer = $url;
	$url = get_url_login($html);

	// get login page
	if ($debug)
		echo "Requesting login URL from: $url\n";
	$html = request($url, $cookie, $referer);
	if ($debug)
		echo "cookie:\n" . print_r($cookie, true) . "\n";

	// get account url
	$referer = $url;
	$url = get_url_baselogin();
	$post = get_post_login($html, $userid, $pin);
	if ($debug)
		echo "Posting:\n" . print_r($post, true) . " to: $url\n";
	$html = request($url, $cookie, $referer, $post);
	if ($debug)
		echo "cookie:\n" . print_r($cookie, true) . "\n";
	$referer = $url;
	$url = get_url_account($html);

	// get account page
	if ($debug)
		echo "Requesting account from: $url\n";
	$html = request($url, $cookie, $referer);
	if ($debug)
		echo "cookie:\n" . print_r($cookie, true) . "\n";

	for ($i = 1; $i <= ($debug ? 1 : 37); $i++) {
		// get month
		$url = get_url_core();
		$referer = $url;
		$post = get_post_transaction($html, $i);
		if ($debug)
			echo "Posting:\n" . print_r($post, true) . " to: $url\n";
		$html = request($url, $cookie, $referer, $post);

		// get csv
		$referer = $url;
		$url = get_url_csv($html);
		$csv = request($url, $cookie, $referer);
		$csv = trim_first_line($csv);
		$csv = utf8_encodei($csv);
		$csv .= "\r\n";
		echo $csv;
		$data .= $csv;
	}
	message_end();
	return $data;
}
?>
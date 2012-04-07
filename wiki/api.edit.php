<?php
// mediawiki API interface
// (C) John Peterson, GNU GPL 3
include_once('api.common.php');
header('content-type: text/plain; charset: utf-8');
class mw_edit {
static function get_article_name($s) {
	return str_replace(' ', '_', $s);
}

static function login(&$cookie) {
	$url = "http://localhost/w/api.php";
	$post = array("format"=>"xml", "action"=>login, "lgname"=>"WikiSysop", "lgpassword"=>mw::$pw);
	$result = mw_cm::curl_request($url, $cookie, '', $post);
	$content = mw_cm::xml2arr($result);
	// print_r($content[1]);
	$token = $content[1]['attributes']['TOKEN'];
	$post = array("format"=>"xml", "action"=>"login", "lgname"=>"WikiSysop", "lgpassword"=>mw::$pw, "lgtoken"=>$token);
	$result = mw_cm::curl_request($url, $cookie, '', $post);
	$content = mw_cm::xml2arr($result);
	// print_r($content[1]);
}

static function edit_token(&$cookie) {
	$url = "http://localhost/w/api.php?format=xml&action=query&prop=info&intoken=edit&titles=Test";
	$result = mw_cm::curl_request($url, $cookie);
	$content = mw_cm::xml2arr($result);
	// print_r($content[3]);
	return $content[3]['attributes']['EDITTOKEN'];
}

static function edit($article, $data) {
	self::login($cookie);
	$token = self::edit_token($cookie);
	$article = self::get_article_name($article);
	$url = "http://localhost/w/api.php";
	$post = array("format"=>"xml", "action"=>"edit", "title"=>$article, "text"=>$data, "token"=>$token);
	$result = mw_cm::curl_request($url, $cookie, '', $post);
	$content = mw_cm::xml2arr($result);
	print_r($content[1]);
}
}
?>
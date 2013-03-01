<?php
// mediawiki API interface
// © John Peterson. License GNU GPL 3.
include_once('api.edit.php');
include_once('api.info.php');
class mw {
	static $pw;
	static public function edit($title, $data) {
		return mw_edit::edit($title, $data);
	}
	static public function info() {
		return mw_info::info();
	}
}
?>
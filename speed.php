#!/usr/bin/env php
<?php
// WAN speedtest.
// © John Peterson. License GNU GPL 3.

include('common/common.php');

$argv[0] = basename($argv[0]);
$tmp = "/tmp/speedtest.jpg";
$home = "${_SERVER['HOME']}/.speedtest";
$savefile = "$home/speedtest.log";
$serverfile = "$home/servers.xml";
$rand = mt_rand(100000000000, 9999999999999);
for ($i = 500; $i <=4000; $i+=500) $size_a[] = $i;
$usage = "Usage: ${argv[0]} serverid [sizeid]\n\tserverid\t'id' attribute in $serverfile.\n\tsizeid\ttransfer size id. Accepted values: 0-7 (500 kB to 30 MB). Default 3.";

// exceptions
if (!isset($argv[1])) { echo $usage.PHP_EOL; return; } else $sid = $argv[1];
if (isset($argv[2])) $size=$argv[2]; else $size=3;
if (!is_numeric($sid)) { echo "$sid is not a valid server id\n"; return; }
if (!is_numeric($size) || $size<0 || $size>7) { echo "$size is not a valid transfer size id.\n"; return; }
if (!read_servers($serverfile, $sid, $server)) { printf("Server %d not found in $serverfile.\n", $sid); return; }

// save
function save($in, $out) {
	global $savefile, $ping;
	$time = date('c');
	file_put_contents($savefile, "$time|$in|$out|$ping\n", FILE_APPEND);
}

// servers
function read_servers($serverfile, $sid, &$server) {
	$s = file_get_contents($serverfile);
	if (!$s) {
		printf("$serverfile not found.\n");
		return false;
	}
	$x = new SimpleXMLElement($s);
	$result = $x->xpath("//server[contains(@id, $sid)]");
	if (sizeof($result) != 1) return false;
	$server = $result[0]->attributes();
	return true;
}

// download
function download($size) {
	global $server, $tmp, $do_server, $server, $iface, $rand, $size_a;	
	$path = dirname($server['url']);
	$w = $size_a[$size];
	$fp = fopen($tmp, 'w+');
	$h = curl_init("$path/random${w}x$w.jpg?x=$rand-$size");
	curl_setopt($h, CURLOPT_HEADER, true);
	curl_setopt($h, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($h, CURLOPT_TIMEOUT, 50);
	curl_setopt($h, CURLOPT_FILE, $fp);
	curl_setopt($h, CURLOPT_FOLLOWLOCATION, true);
	$ts = microtime(true);
	$response = curl_exec($h);
	$time = microtime(true)-$ts;
	curl_close($h);
	fclose($fp);
	$in = filesize($tmp);	
	$speed = number_format(round((($in*8)/pow(10,6))/$time,2),2);	
	if ($in < 1<<10) {
		print "$response\n";
		print file_get_contents($tmp);
	}
	return $speed;	
}
function upload(){
	global $server, $tmp, $do_server, $server, $iface, $rand;
	$h = curl_init();
	curl_setopt($h, CURLOPT_HEADER, 0);
	curl_setopt($h, CURLOPT_VERBOSE, 0);
	curl_setopt($h, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($h, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible;)");
	curl_setopt($h, CURLOPT_URL, "${server['url']}?x=0.$rand");
	curl_setopt($h, CURLOPT_POST, true);
	$post = array("file_box"=>"@$tmp");
	curl_setopt($h, CURLOPT_POSTFIELDS, $post);
	$ts = microtime(true);
	$response = curl_exec($h);
	$time = microtime(true)-$ts;
	$headsize = substr($response, 5);
	$out = filesize($tmp)+$headsize;	
	$speed = number_format(round((($out*8)/pow(10,6))/$time,2),2);
	return $speed;
}

// main
$ping = ping(parse_url($server['url'],PHP_URL_HOST));
$distance = round(distance(a2loc(get_address()),array(trim($server['lat']),trim($server['lon']))));
$in = download($size);
$out = upload();
save($in, $out);
@unlink($tmp);
printf("↓ $in Mb/s\t↑ $out Mb/s\t↔ %s ms\t %s km\t%s, %s\n", $ping, $distance, $server['name'],$server['country']);
?>
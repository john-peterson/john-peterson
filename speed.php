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
$locationfile = "$home/location.txt";
$serverfile_url = "http://pastebin.com/raw.php?i=XcpejEar";
$rand = mt_rand(100000000000, 9999999999999);
for ($i = 500; $i <=4000; $i+=500) $size_a[] = $i;
$usage = "Usage: ${argv[0]} serverid [sizeid]\n\tserverid\t'id' attribute in $serverfile.\n\tsizeid\ttransfer size id. Accepted values: 0-7 (500 KiB to 30 MiB). Default 3.";

// exceptions
if (!isset($argv[1])) { usage(); return; } else $sid = $argv[1];
if (isset($argv[2])) $size=$argv[2]; else $size=3;
if (!is_numeric($sid)) { echo "$sid is not a valid server id\n"; return; }
if (!is_numeric($size) || $size<0 || $size>7) { echo "$size is not a valid transfer size id.\n"; return; }
if (!read_servers($sid, $server)) { printf("Server %d not found in $serverfile.\n", $sid); return; }

// print usage
function usage() {
	global $serverfile, $usage;
	echo "$usage\n";
	echo PHP_EOL.adjacent_servers();
}

// save
function save($in, $out) {
	global $savefile, $ping;
	$time = date('c');
	file_put_contents($savefile, "$time|$in|$out|$ping\n", FILE_APPEND);
}

// location
function get_location() {
	global $locationfile;
	$l = trim_w(@file($locationfile));
	if (sizeof($l) != 2) {
		$l = a2loc(get_address());
		file_put_contents($locationfile, implode("\n", $l));
	}
	if (!$l) {
		printf("$locationfile not found and could not obtain location.\n");
		return false;
	}
	return $l;
}

// servers
function read_serverfile() {
	global $serverfile, $serverfile_url;
	$s = @file_get_contents($serverfile);
	if (!$s) {
		printf("$serverfile not found, downloading.\n");
		$s = file_get_contents($serverfile_url);
	}
	if ($s)
		file_put_contents($serverfile, $s);
	else {
		printf("$serverfile not found and could not be downloaded.\n");
		return false;
	}
	return $s;
}
function read_servers($sid, &$server) {
	global $serverfile;
	if (!($s = read_serverfile($serverfile))) return;
	$x = new SimpleXMLElement($s);
	$result = $x->xpath("//server[contains(@id, $sid)]");
	if (sizeof($result) != 1) return false;
	$server = $result[0]->attributes();
	return true;
}
// adjacent servers
function adjacent_servers() {
	global $serverfile;
	if (!($location = get_location())) return;
	if (!($s = read_serverfile($serverfile))) return;

	$x = new SimpleXMLElement($s);
	foreach (xml2array($x)['children'][0]['children'] as $v) {
		$v = $v['attributes'];
		$lat = trim($v['lat']);
		$lon = trim($v['lon']);
		$distance = round(distance($location, array($lat,$lon)));
		$servers[$distance]['name'] = "{$v['name']}, {$v['country']}";
		$servers[$distance]['id'] = $v['id'];
	}
	// sort by distance
	ksort($servers);
	$i = 0;
	echo "\nYour closest servers\n\ndistance (km)\tname\tid\n";
	foreach ($servers as $k => $s) {
		echo "$k\t{$s['name']}\t{$s['id']}\n";
		$i++;
		if ($i > 10) break;
	}
	exit;
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
	$speed = number_format(round((($in)/0x100000)/$time,2),2);
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
	$speed = number_format(round((($out)/0x100000)/$time,2),2);
	return $speed;
}

// main
$ping = ping(parse_url($server['url'],PHP_URL_HOST));
$distance = round(distance(get_location(), array(trim($server['lat']),trim($server['lon']))));
$in = download($size);
$out = upload();
save($in, $out);
@unlink($tmp);
printf("↓ $in MiB/s\t↑ $out MiB/s\t↔ %s ms\t %s km\t%s, %s\n", $ping, $distance, $server['name'],$server['country']);
?>
#!/usr/bin/env php
<?php
// justin.tv speed test.
// © John Peterson. License GNU GPL 3.

include('common/common.php');

$argv[0] = basename($argv[0]);
$br = array(20000,10000,5000,3000,2000,1000);
$sec = 30;
$out = "/tmp/jtv.flv";
$lfo = "/tmp/avconv.out.log"; $lfi = "/tmp/avconv.in.log"; $lr = "/tmp/rtmpdump.log";
$STDERR = fopen('php://stderr', 'w+');
$usage = sprintf("Usage: %s [-h] [-V] -c chan [-t stream] -v file [-S s] -s server [-b bitrate(s)] [-j]\n", $argv[0]);
$opts = "hVc:t:v:S:s:b:j";
$opts_long  = array("help", "verbose", "channel:","stream:","video:","seconds:","server:","bitrate:","jtv");
$args = getopt($opts, $opts_long);
foreach ($args as $key => $arg) {
	switch ($key) {
		case 'h':
		case 'help':
			echo $usage;
			exit(0);
		case 'V':
		case 'verbose':
			$debug = true;
			break;
		case 'c':
		case 'channel':
			$c = $arg;
			break;
		case 't':
		case 'stream':
			$stream = $arg;
			break;
		case 'v':
		case 'video':
			$v = $arg;
			break;
		case 'S':
		case 'seconds':
			$sec = $arg;
			break;
		case 's':
		case 'server':
			$server = $arg;
			break;
		case 'b':
		case 'bitrate':
			$br = explode(" ", $arg);
			break;
		case 'j':
		case 'jtv':
			$j = true;
			break;
		default:
			echo sprintf("unrecognised argument %s\n", $key);
	}
}
// exceptions
if (!file_exists($v)) { log0("'%s' not found", $v); return; }

function log1() {
	global $STDERR, $debug;
	if (!$debug) return;
	$arg_list = func_get_args();
	$s = $arg_list[0];
	array_shift($arg_list);
	fwrite($STDERR, vsprintf($s, $arg_list));
}
function log0() {
	global $STDERR;
	$arg_list = func_get_args();
	$s = $arg_list[0];
	array_shift($arg_list);
	fwrite($STDERR, vsprintf($s, $arg_list));
}

function find_line($s, $t) {
	$s = substr($s, strripos($s, $t));
	return substr($s, 0, strripos($s, "\n"));
}

function jtv($c, $stream = "") {
	$xml = file_get_contents("http://usher.justin.tv/find/$c.xml?type=any");
	// $xml = file_get_contents("jtv.xml");
	if ($xml == '<nodes></nodes>') { print "$c is offline\n"; return false; }
	# fix invalid xml
	$xml = preg_replace('/<\/?\K(?=\d+)/', '_$1', $xml);
	$player = explode(' ', shell_exec("curl -v 'http://www-cdn.justin.tv/widgets/live_site_player.swf' 2>&1"))[48];
	if (isset($stream) && count(xpath($xml, "//$stream/token")) > 0)
		$path = $stream;
	else
		$path = "*[not(video_height < //*[token]/video_height) and token][1]";
	// print streams
	$y = xpath($xml, "name($path)");
	for ($i = 1; $i <= count(xpath($xml, "*[token]")); $i++) {
		$x = xpath($xml, "name(*[token][$i])");
		if ($x == $y) print "*";
		print $x."\n";
	}
	$token =  xpath($xml, "$path/token");
	$connect = xpath($xml, "$path/connect");
	$play = xpath($xml, "$path/play");
	return array($player, $token, $connect, $play);
}

// frame count, duration
function io($f, $dur = false) {
	global $out;
	$s = str_replace("\r", "\n", file_get_contents($f));
	log1("%s", $s);
	$s = find_line($s, "frame=");
	preg_match_all("/(?>frame|size|time)=\s*([\d.]+)/", $s, $res);
	$res = $res[1];
	if ($dur) $res[2] = shell_exec("mediainfo --Inform='General;%Duration%' '$out'") / 1000;
	return sprintf("%s kiB, %s kiB/s, %s kb/s, %d f, %s f/s", number_format($res[1]), number_format($res[1]/$res[2],1), number_format((($res[1]/1024)*8000)/$res[2]), $res[0], number_format($res[0]/$res[2],1));
}

foreach ($br as $b) {
	log0("Streaming %s kb/s for %d s ...\n", number_format($b), $sec);
	shell_exec("nohup avconv -re -i '$v' -t $sec -vcodec libx264 -preset fast -vb ${b}k -strict -2 -acodec aac -ab 256k -ac 2 -f flv $server 1>&2 2>$lfo &");
	
	// wait for stream to come online
	if (!$j)
		for ($i = 0; $i < 5; $i++) { if (!empty(xpath(file_get_contents("http://".parse_url($server,PHP_URL_HOST).":8086/connectioncounts"), "//Application"))) break; sleep(1); if ($i == 4) { print "Stream offline\n"; return; } }
	else
		for ($i = 0; $i < 5; $i++) { if (strlen(file_get_contents("http://usher.justin.tv/find/$c.xml?type=any")) > 15) break; sleep(1); if ($i == 4) { log0("Stream offline\n"); return; } }
	if ($j) {
		list($player, $token, $connect, $play) = jtv($c, $stream);
		if (strlen($token) == 0) { log0("Could not retrieve token\n"); return; }
	}
	log0("Stream online ... \n");
	
	if (!$j)
		$pid = shell_exec("nohup rtmpdump -vr'$server' -b0 -o$out >$lr 2>&1 & echo -n $!");
	else
		$pid = shell_exec("nohup rtmpdump -vr'$connect/$play' -b0 -j'$token' -W'$player' -o'$out' >$lr 2>&1 & echo -n $!");
	log1("Download pid %s ...\n", $pid);
	$i=0;
	while (true) {
		unset($o);
		exec('ps -p'.$pid, $o);
		log0("\r\033[KDownloading %d ...", $i);	
		if (count($o) < 2) break;
		$i++;
		sleep(1);
	}
	log0("\n");
	log1("%s\n", file_get_contents($lr));
	log0("↑ %s\n", io($lfo));
	$o = shell_exec("avconv -i '$out' -vcodec copy -acodec copy -f flv - 1>/dev/null 2>$lfi");
	log1("%s\n", $o);
	log0("↓ %s\n", io($lfi,true));
	$host = parse_url($server,PHP_URL_HOST);
	// $ping = ping($host);
	// $distance = round(distance(a2loc(get_address()),a2loc(gethostbyname($host))));
	// log0("↔ %s ms\t %s km\n", $ping, number_format($distance));
}
?>
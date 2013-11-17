#!/usr/bin/env php
<?php
// justin.tv speed test.
// © John Peterson. License GNU GPL 3.

include('common/common.php');

$argv[0] = basename($argv[0]);
$out = "/tmp/jtv.flv";
$lfo = "/tmp/avconv.out.log"; $lfi = "/tmp/avconv.in.log";
$lr = "/tmp/rtmpdump.log";
$STDERR = fopen('php://stderr', 'w+');
$usage = sprintf("Usage: %s [-h] [-V] -c chan [-t stream] -v file [-S s] -s server [-b bitrate(s)]\n", $argv[0]);

// default arguments
$stream = "best";
$br = array(100,1000,2000,3000,5000,10000,20000);
$sec = 30;

// parse arguments
$opts = "hVc:t:v:S:s:b:";
$opts_long  = array("help", "verbose", "channel:", "stream:", "video:", "seconds:", "server:", "bitrate:");
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
		default:
			echo sprintf("unrecognised argument %s\n", $key);
	}
}

// exception
if (!file_exists($v)) {
	log0("'%s' not found", $v);
	return;
}

function log1() {
	global $STDERR, $debug;
	if (!$debug)
		return;

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

function stream($c, $stream) {
	$json = shell_exec("livestreamer $c $stream --stream-priority=rtmp --json");
	
	# parse error
	$error = json_decode($json, true)['error'];
	if (strlen($error) > 0) {
		log1("%s\n", $error);
		return false;
	}

	# parse parameters
	$rtmp = json_decode($json, true)['params']['rtmp'];
	$playpath = json_decode($json, true)['params']['playpath'];
	$swfUrl = json_decode($json, true)['params']['swfUrl'];
	$jtv = json_decode($json, true)['params']['jtv'];

	return array($rtmp, $playpath, $swfUrl, $jtv);
}

// frame count, duration
function io($f, $dur = false) {
	global $out;
	$s = str_replace("\r", "\n", file_get_contents($f));
	log1("%s", $s);
	$s = find_line($s, "frame=");
	preg_match_all("/(?>frame|size|time)=\s*([\d.]+)/", $s, $res);
	$res = $res[1];
	if ($dur)
		$res[2] = shell_exec("mediainfo --Inform='General;%Duration%' '$out'") / 1000;
	return sprintf("%s kiB, %s kiB/s, %s kb/s, %d f, %s f/s", number_format($res[1]), number_format($res[1]/$res[2],1), number_format((($res[1]/1024)*8000)/$res[2]), $res[0], number_format($res[0]/$res[2],1));
}

foreach ($br as $b) {
	log0("Streaming %d s at %s kb/s ...\n", $sec, number_format($b));
	shell_exec("nohup avconv -re -i '$v' -t $sec -vcodec libx264 -preset fast -vb ${b}k -strict -2 -acodec aac -ab 256k -ac 2 -f flv $server 1>&2 2>$lfo &");
	
	// wait for stream to come online
	for ($i = 0; $i < 5; $i++) {
		$svr = stream($c, $stream);
		if ($svr)
			break;
		sleep(1);
		if ($i == 4) {
			log0("Stream offline\n");
			return;
		}
	}
	list($rtmp, $playpath, $swfUrl, $jtv) = $svr;
	log1("Stream online ...\n");

	// download stream
	if (strlen($playpath) > 0)
		$rtmp = "$rtmp/$playpath";
	$pid = shell_exec("nohup rtmpdump -r '$rtmp' -s '$swfUrl' -j '$jtv' -v -b0 -o '$out' >$lr 2>&1 & echo -n $!");
	$i = 0;
	while (true) {
		unset($o);
		exec('ps -p'.$pid, $o);
		log0("\r\033[KDownloading %d s ...", $i);
		if (count($o) < 2)
			break;
		$i++;
		sleep(1);
	}

	// print result
	log0("\n");
	log1("%s\n", file_get_contents($lr));
	log0("↑ %s\n", io($lfo));
	$o = shell_exec("avconv -i '$out' -vcodec copy -acodec copy -f flv - 1>/dev/null 2>$lfi");
	log1("%s\n", $o);
	log0("↓ %s\n", io($lfi, true));

	// ping and distance
	// $host = parse_url($server,PHP_URL_HOST);
	// $ping = ping($host);
	// $distance = round(distance(a2loc(get_address()), a2loc(gethostbyname($host))));
	// log0("↔ %s ms\t %s km\n", $ping, number_format($distance));
}
?>
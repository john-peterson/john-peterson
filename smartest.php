smartest smartctl S.M.A.R.T. error ddrescue marked sectors test
(C) John Peterson, licensed under GNU GPL 3
<?php
$argv[1];
$argv[2];
$usage = 'Usage: php smartest.php command logfile';

// Exceptions
if (!isset($argv[2])) { echo PHP_EOL.$usage. HP_EOL; return; }
if (!file_exists($argv[2])) { PHP_EOL.$argv[2] . ' not found'.PHP_EOL; return; }
if (!preg_match('/^ddrescue.*( \/dev\/\w+).*(\/dev\/\w+).*/', $argv[1]))
	{ echo 'Unsupported command: ' . $argv[1] . PHP_EOL; return; }
		
// Parse input
function parse_err($lines) {
	$log_arr = array();
	$first_log = false;
	foreach ($lines as $i => $line) {
		if (strpos($line, 'occurred at disk power-on lifetime')) {
			if ($first_log) $log_arr[$num] = $log;
			$num = explode(' ', $line)[1];
			$first_log = true;
			$log = "";
		}
		else if (strpos($line, 'Error: UNC') && strpos($line, 'sectors at LBA')) {				
				$lba[] = array(rtrim(explode(' ', $line)[25]), explode(' ', $line)[18]);
			}
		else
			if ($first_log && preg_match('[^\w]', $line)) goto ret;
		if ($first_log) $log .= $line;		
	}
	ret:
	if (strlen($log)) $log_arr[$num] = $log;
	return array($log_arr, $lba);
}

// Read logfile
list ($log, $lba) = parse_err(file($argv[2]));
// Call ddrescue
$cmd = explode('/', $argv[1], 2); $cmd[1] = '/' . $cmd[1];
foreach ($lba as $i => $row) {
	$_cmd = sprintf("%s %s %s %s", rtrim($cmd[0]), '-i'.$row[0].'b', '-s'.$row[1].'b', $cmd[1]);
	echo PHP_EOL.'$ '.$_cmd;
	passthru($_cmd);
}
?>
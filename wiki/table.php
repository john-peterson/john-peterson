<?php
// wiki table editor
// (C) John Peterson, GNU GPL 3
function roundn($val, $dec) {
	return ltrim(round($val, $dec), '0');
}

function trim_arr(&$value) {
	$value = trim($value);
}

function parse_int($string) {
	if(preg_match('/(\d+)/', $string, $array))
		return $array[1];
	else
		return 0;
}

function multi_explode($delimiters = array(), $string = '') {
    $main_delim = $delimiters[count($delimiters)-1];
    array_pop($delimiters);    
    foreach ($delimiters as $delimiter) {    
        $string = str_replace($delimiter, $main_delim, $string); 	
    }
	$string = trim(preg_replace('/\s+/', ' ', $string));
    $result = explode($main_delim, $string); 
    return $result;
}

function diag2ppcm($d, $r) {
	list($pw, $ph) = explode('×', $r);
	$dp = sqrt($pw*$pw+$ph*$ph);
	return $dp/$d;
}

function span_decrement(&$span) {
	foreach ($span as &$elem) {
		if ($elem[0] > 0) $elem[0]--;
	}
}

function span_update($elem, &$span, $len) {
    for ($i = 0, $j = 0; $i < $len, $j < sizeof($elem); $i++, $j++) {
		if ($span[$i][0] > 0) {
			$j--;
			continue;
		}
		$span[$i][0] = $span[$i][1] = 0;
		if (preg_match('/rowspan/', $elem[$j]))
			$span[$i][0] = $span[$i][1] = parse_int(strstr($elem[$j], '|', true));
    }
}

function span_adjust(&$col, $span) {
	$_col = $col;
    foreach ($_col as $elem) {
        if ($span[$elem][0] > 0 && $span[$elem][0] != $span[$elem][1]) {
			foreach ($col as $key => &$e) {
				if ($e == $elem) $e = false;
				if ($e > $elem) $e--;				
			}
		}
    }	
}

function col_create($elem, $find, &$col, &$data) {
	$col = array(); $data = array();
	foreach ($find as $f) {
		if (array_search($f, $elem) !== false)
			$col[$f] = array_search($f, $elem);
	}
}

function data_update($col, &$data, &$note, $elem) {
	foreach ($col as $key=>$val) {
		$d = $elem[$col[$key]];
		if (strpos($d,'|')) $d = substr($d,strpos($d,'|')+1);
        if ($col[$key] !== false) {
			$data[$key] = trim_nonnum($d);
			$note[$key] = trim_note($d);
		}
		if ($col[$key] >= sizeof($elem)) $data[$key] = false;
    }
}

// parse table
function parse_wiki($lines) {
	$wiki = "";
	$span = array();
	foreach ($lines as $i=>$line) {
		$line = replace_table_text($line);
		if (!strncmp($line, '!', 1)) {
			$_line = trim(substr($line, 1, strlen($line)-1));
			$elem = explode('!!',$_line);
			$len = sizeof($elem);
			array_walk($elem, 'trim_arr');
			col_create($elem, array('Diagonal','Resolution','PPI','Width','Height'), $_col, $data);
			// replace text
			if (isset($_col['Diagonal'])) $elem[$_col['Diagonal']] = 'Diagonal cm (in)';
			if (isset($_col['PPI'])) $elem[$_col['PPI']] = 'ppcm (PPI)';
			if (isset($_col['Width'])) $elem[$_col['Width']] = 'Width cm (in)';
			if (isset($_col['Height'])) $elem[$_col['Height']] = 'Height cm (in)';
			$line = '!'.implode(' !! ', $elem).PHP_EOL;
		} else if (!strncmp($line, '|', 1)
		&& strncmp($line, '|-', 2)
		&& strncmp($line, '|+', 2)
		&& strncmp($line, '|}', 2)) {
			$line = substr($line, 1, strlen($line)-1);
			$line = replace_elem_text($line);
			$elem = explode('||',$line);
			array_walk($elem, 'trim_arr');
			span_decrement($span);
			span_update($elem, $span, $len);
			$col = $_col;
			span_adjust($col, $span);
			data_update($col, $data, $note, $elem);
			// replace text
			if (isset($data['Diagonal']) && $data['Diagonal'] !== false && !($span[$col['Diagonal']][0] && $span[$col['Diagonal']][0] != $span[$col['Diagonal']][1])) $elem[$col['Diagonal']] = ($span[$col['Diagonal']][0]>0 ? 'rowspan="'.$span[$col['Diagonal']][1].'" | ' : '').'{{convert|'.$data['Diagonal'].'|in|cm|abbr=values|disp=flip}}';
			if (isset($data['PPI']) && $data['PPI'] !== false) $elem[$col['PPI']] = '{{convert|'.round($data['PPI']).'|cm|in|abbr=values|disp=flip}}'.$note['PPI'];
			if (isset($data['Width']) && $data['Width'] !== false) $elem[$col['Width']] = '{{convert|'.$data['Width'].'|in|cm|abbr=values|disp=flip}}';
			if (isset($data['Height']) && $data['Height'] !== false) $elem[$col['Height']] = '{{convert|'.$data['Height'].'|in|cm|abbr=values|disp=flip}}';
			$line = '|'.implode(' || ', $elem).PHP_EOL;
		}
		$line = replace_text($line);
		$wiki .= $line;
	}
	return $wiki;
}

// replace text
function trim_nonnum($s) {
	return preg_replace('/[^\d.×]/', '', $s);
}
function trim_nonanum($s) {
	return preg_replace("/[^A-Za-z0-9×]/", "", $s);
}
function trim_note($s) {
	return preg_replace('/[^\*]/', '', $s);
}
function replace_elem_text($s) {
	$s = preg_replace( '/([0-9]+)x([0-9]+)/i', '$1×$2', $s);
	return preg_replace( '/([0-9]+)\*([0-9]+)/i', '$1×$2', $s);
}
function replace_table_text($s) {
	$s = str_replace('PQ 3Qi-01 sunlit || 10.1" || 3072×600 || 332×107', 'PQ 3Qi-01 sunlit || 10.1" || 3072×600 || '.round(diag2ppcm('10.1', '3072×600')), $s);
	$s = str_replace('PQ 3Qi-01 backlit || 10.1" || 1024×600 || 111×107', 'PQ 3Qi-01 backlit || 10.1" || 1024×600 || '.round(diag2ppcm('10.1', '1024×600')), $s);
	return $s;
}
function replace_text($s) {
	$s = str_replace('pixels per inch (PPI)', 'pixels per cm (ppcm) (pixels per inch (PPI))', $s);
	return $s;
}

// save article
function save_article($a, $f, $force = false) {
	$a = str_replace(' ', '_', $a);
	if (!file_exists($f) || $force) {
		$file = "http://en.wikipedia.org/w/index.php?action=raw&title=$a";
		$opts = array('http'=>array('method'=>"GET", 'header'=>"User-Agent: PHP script"));
		$context = stream_context_create($opts);
		$file = file_get_contents($file, false, $context);
		file_put_contents($f, $file);
	}
}

// article
$f = 'article.txt';
$g = 'article.edit.txt';
$title = 'List of displays by pixel density';
save_article($title, $f);
$data = parse_wiki(file($f));
file_put_contents($g, $data);

// preview
include('api.php');
mw::$pw = '';
mw::edit($title, $data);

echo $g." saved.";
?>
## File tools

**sparse** Scans a file for 0x00 blocks and create sparse blocks in their place.
 
	sparse [/s] disk.vmdk
	/s	Set sparse flag. Otherwise show sparse flag and size.

**sfcex** Scans a System File Checker (`sfc`) log for uncorrectable files (both the file and the source store copy is damaged) and replace them from a Windows Imaging Format (wif) Windows installation file or a Windows installation file system.
 
	sfcex /d:z /w:x:4 CBS.log	# copy from x: wif (Windows edition 4 in x:/sources/install.wim/1.xml)
	sfcex /d:z /s:x CBS.log		# copy from x:/windows

**csumtree** Recursive folder comparison. List, or write to, missing or changed files.

	csumtree d:\Windows e:\Windows

### S.M.A.R.T. tools

**smarterr** Read error output from `smartctl` and append new errors to a file. (Useful because the disk only retain the recent errors and it can be of interest to have a more complete log of errors.)

	smarterr.php "smartctl --log xerror,99 d:" smarterr.log
	schtasks /Create /F /TN "User\smarterr" /SC DAILY /ST 03:00 "hstart /NOCONSOLE \"php \"C:\...\smarterr.php\" \"smartctl -d sat --log xerror,99 p:\" \"C:\...\smarterr.log\"\""		# example scheduling

**smartest** Read UNC errors from smarterr output and read failed sectors with ddrescue.

	smartest.php "ddrescue -vfdM /dev/sdb /dev/null ddrescue.log" smarterr.log

## Network tools

**speed.php** Measure roundtrip time and throughput to a speedtest.net server.

	speed.php			# print the ten closest servers
	speed.php 1385 7	# use server 1385, transmit 30 MiB (size 7) up and down for the test

**speed.stream.php** Measure roundtrip throughput with a justin.tv media server. Currently only a unidirectional throughput test is avaliable (in XSplit) and the bidirectional rate can differ significantly from the unidirectional rate.

	speed.stream.php -v"$v" -s'rtmp://media/live/stream' -S30 -b'20000 10000 5000 1000'	# stream the file $v for 30 s for each of the specified bitrates

## Media tools

**wplsync** Sync playlists to directory. Useful since WMP only sync to devices.

	wplsync.php "C:\Music\Playlists" "D:\Music"

**bing** Download the 1920x1200 version of bing.com's daily image.
  
	bing.sh		# download to .

## Development tools

**diffdiff** Assist in comparing diff files. Files unique to one of the diff files are excluded.

	diffdiff.php diff1.patch diff2.patch

## Windows tools

**envupdate** Update environment for all processes with a message loop (send `WM_SETTINGCHANGE`).

	envupdate

**SndVol** Makes `SndVol.exe` (Windows Volume Mixer) resizable by changing its `GWL_STYLE` and overriding its `WindowProc`.

	SndVol --wait	# keep loader window open to allow pinning to taskbar

**mmsys** Makes `mmsys.cpl` resizable by changing its `GWL_STYLE` and overriding its `WindowProc`.

	mmsys

**fullscreen** Creates a system-global hotkey that change the window size of a specified window. The implemented application is for the Playstation emulator pSX to fix the aspect ratio and remove black borders in Final Fantasy and other games.

![alt text](https://raw.github.com/john-peterson/john-peterson/master/fullscreen.png "fullscreen")

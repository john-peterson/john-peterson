#!/bin/bash
# justin.tv continuous playback.
# (C) John Peterson. License GNU GPL 3.

if [ -z "$1" ]; then echo -e "Usage: jtv.sh channel [stream]"; exit 0; fi
channel=$(echo $1|perl -e'print lc <>;')
stream=$2
echo >~/.readb

{ n=0; while :; do
xml=$(curl -s "http://usher.justin.tv/find/$channel.xml?type=any")
if [ "$xml" = "<nodes></nodes>" ]; then echo "$channel is offline" 1>&2; break; fi
player=$(curl -v "http://www-cdn.justin.tv/widgets/live_site_player.swf" 2>&1|grep Location:|cut -d" " -f3|cut -d\? -f1)
# fix invalid xml
xml=$(echo $xml|sed 's/<\([0-9]\)/<_\1/g'); xml=$(echo $xml|sed 's/<\/\([0-9]\)/<\/_\1/g')
# stream selection
if [ -n "$stream" ] && [ -n "`echo $xml|xmllint --xpath '//$stream/token' -`" ] ; then path="$stream"
else path="*[not(video_height < //*[token]/video_height) and token][1]"; fi
# print streams
y=`echo $xml|xmllint --xpath "name($path)" -`
for ((i=1;i<=$(echo $xml|xmllint --xpath "count(*[token])" -);i++)); do
	x=$(echo $xml|xmllint --xpath "name(*[token][$i])" -)
	if [ "$x" = "$y" ]; then echo -n "*" 1>&2; fi
	echo "$x" 1>&2
done
echo 1>&2
token=$(echo $xml|xmllint --xpath "$path/token/text()" -)
connect=$(echo $xml|xmllint --xpath "$path/connect/text()" -)
play=$(echo $xml|xmllint --xpath "$path/play/text()" -)
if [ -z "$token" ] || [ -z "$connect" ] || [ -z "$play" ]; then printf "Incomplete usher\n" 1>&2; break; fi
if [ $n -eq 0 ]; then
	rtmpdump -vr"$connect/$play" -j"$token" -W"$player" -m5 -b0 -o-|readb -t5 -s"rtmpdump"
	q=$?
	echo $q 1>&2
else
	rtmpdump -vr"$connect/$play" -j"$token" -W"$player" -m5 -b0 -o-|readb -ht5 -s"rtmpdump"
	q=$?
	echo $q 1>&2
fi
if [ $q -eq 141 ]; then break; fi
((n++))
echo Restarting ... 1>&2
done; }|vlc -q -
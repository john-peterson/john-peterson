#!/bin/bash
# justin.tv player.
# (C) John Peterson. License GNU GPL 3.
while getopts ":io:p:" c; do
	case $c in
	i) info=true;;
	o) out=$OPTARG;;
	p) prog=$OPTARG;;
	\?) echo "Invalid option: -$OPTARG" >&2; exit 1;;
	esac
done
shift $(($OPTIND-1))
channel=$(echo $1|perl -e'print lc <>;')
stream=$2
if [ -z "$channel" ]; then echo -e "Usage: jtv.sh [-i] [-o output] [-p program] channel [stream]\n-i\tonly show stream information.\n-o\toutput. example: \">file\". default \"|vlc -\".\n-p\tprogram. default rtmpdump."; exit 0; fi
xml=$(curl -s "http://usher.justin.tv/find/$channel.xml?type=any")
if [ "$xml" = "<nodes></nodes>" ]; then echo "$channel is offline"; exit 1; fi
player=$(curl -v "http://www-cdn.justin.tv/widgets/live_site_player.swf" 2>&1|grep Location:|cut -d" " -f3|cut -d\? -f1)
# fix invalid xml
xml=$(echo $xml|sed 's/<\([0-9]\)/<_\1/g'); xml=$(echo $xml|sed 's/<\/\([0-9]\)/<\/_\1/g')
# select stream
if [ -n "$stream" ] && [ -n "`echo $xml|xmllint --xpath '//${stream}/token' -`" ]; then path="$stream"
else path="*[not(video_height < //*[token]/video_height) and token][1]"; fi
# print streams
y=`echo $xml|xmllint --xpath "name($path)" -`
for ((i=1; i<=$(echo $xml|xmllint --xpath "count(*[token])" -); i++)); do
	x=$(echo $xml|xmllint --xpath "name(*[token][$i])" -)
	if [ "$x" = "$y" ]; then echo -n "*"; fi
	echo $x
done
echo
token=$(echo $xml|xmllint --xpath "$path/token/text()" -)
connect=$(echo $xml|xmllint --xpath "$path/connect/text()" -)
play=$(echo $xml|xmllint --xpath "$path/play/text()" -)
echo $token
echo $connect
exit 0
if [ -z "$token" ] || [ -z "$connect" ] || [ -z "$play" ]; then printf "Incomplete usher\n"; exit 1; fi
if [ -n "$info" ]; then
	out="|/dev/null"
elif [ -z "$out" ]; then
	out="|vlc -"
fi
if [ -z "$prog" ]; then
	prog="rtmpdump"
fi
eval $prog '-vr"$connect/$play" -j"$token" -W"$player" -o-'$out
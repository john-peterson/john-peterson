#!/bin/bash
# justin.tv player
# (C) John Peterson, license GNU GPL 3
if [ -z "$1" ]; then echo -e "Usage: jtv.sh channel [stream]"; exit 0; fi;
channel=$1
stream=$2
player=$(curl -v "http://www-cdn.justin.tv/widgets/live_site_player.swf" 2>&1|grep Location:|cut -d" " -f3|cut -d\? -f1)
xml=$(curl -s "http://usher.justin.tv/find/${channel}.xml?type=any")
if [ "$xml" = "<nodes></nodes>" ]; then echo "${channel} is offline"; exit 1; fi;
# stream node name form error
xml=$(echo $xml|sed 's/<\([0-9]\)/<_\1/g'); xml=$(echo $xml|sed 's/<\/\([0-9]\)/<\/_\1/g');
# stream selection
if [ -n "${stream}" ] && [ -n "`echo $xml|xmllint --xpath "//${stream}/token" -`" ] ; then path="${stream}";
else path="*[not(video_height < //*[token]/video_height) and token][1]"; fi;
# print streams
y=`echo $xml|xmllint --xpath "name($path)" -`
for ((i=1;i<=$(echo $xml|xmllint --xpath "count(*[token])" -);i++)); do
	x=$(echo $xml|xmllint --xpath "name(*[token][$i])" -)
	if [ "$x" = "$y" ]; then echo -n "*"; fi;
	echo "$x"
done
echo "";
token=$(echo $xml|xmllint --xpath "${path}/token/text()" -)
connect=$(echo $xml|xmllint --xpath "${path}/connect/text()" -)
play=$(echo $xml|xmllint --xpath "${path}/play/text()" -)
rtmpdump -vr"${connect}/${play}" -j"${token}" -W"${player}" -o-|vlc -
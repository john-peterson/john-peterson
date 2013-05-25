#!/bin/bash
# Bing image downloader
# © John Peterson. License GNU GPL 3.
bing_url="http://www.bing.com"
i=0
while :; do
	xml=$(curl -s "${bing_url}/HPImageArchive.aspx?format=xml&idx=${i}&n=1")
	url=$(echo $xml|xmlstarlet sel -t -v '//urlBase/text()')
	description=$(echo $xml|xmlstarlet sel -t -v '//copyright/text()')
	url=${bing_url}${url}_1920x1200.jpg
	file=`basename $url`
	if [ -a ./$file ]; then
		echo "$file already present, skipping"
	else
		echo "curl: $url to $file"
		curl $url -o $file
		if [ $? -gt 0 ]; then echo "Failed: $i"; break; fi
		echo "XMP:Description: $description"
		exiv2 -M"set Xmp.dc.description $description" $file
	fi
	if [ $i -ge 19 ]; then break; fi;
	((i++))
done
#!/bin/bash
# Bing image downloader
# (C) John Peterson, license GNU GPL 3
bingUrl="http://www.bing.com"
i=0
while true; do
	xml=$(curl "http://www.bing.com/HPImageArchive.aspx?format=xml&idx=${i}&n=1")
	url=$(echo $xml|xmllint --xpath '//urlBase/text()' -)
	description=$(echo $xml|xmllint --xpath '//copyright/text()' -)
	url=${bingUrl}${url}_1920x1200.jpg
	fileName=${url##*%2f}
	if [ -a ./$fileName ]; then
		echo "$fileName already found, skipping"
	else
		echo "curl: $url to $fileName"
		curl $url -o $fileName
		if [ "$?" -gt "0" ]; then
			echo "Failed: $i"
			break
		fi
		echo "XMP:Description: $description"
		exiv2 -M"set Xmp.dc.description $description" $fileName
	fi
	((i++))
done
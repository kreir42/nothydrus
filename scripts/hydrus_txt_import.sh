#!/bin/sh
sidecar_extension=.txt
executable=./nothydrus
temp_file=hydrus_txt_import.tmp

touch "$temp_file"
for path in "$@"; do
	case "$path" in
		*"$sidecar_extension")
			;;
		*)
			if [ -e "$path""$sidecar_extension" ]; then
				echo "$path" >> "$temp_file"
			fi
			;;
	esac
done
"$executable" add < "$temp_file"
while read -r path; do
	sed "s/\(.*\)/\"\1\"/;s/:/\" --taggroup \"/" "$path""$sidecar_extension" |
	while read -r line; do
		"$executable" tag "$line" "$path"
	done
done < "$temp_file"
rm "$temp_file"

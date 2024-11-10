#!/bin/sh
sidecar_extension=.txt
executable=./nothydrus
temp_file=hydrus_txt_import.tmp

touch "$temp_file"
touch "$temp_file"2
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
	sed 's/\(.*\)/\"\1\"/' "$path""$sidecar_extension" |
	while read -r line; do
		echo "$executable" tag --add "$line" "$path" >> "$temp_file"2	#TBD workaround, this can probably be done in a more straightforward way
	done
done < "$temp_file"
rm "$temp_file"
sh "$temp_file"2
rm "$temp_file"2

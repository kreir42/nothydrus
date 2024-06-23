#!/bin/sh
rm -rd .nothydrus
make && ./nothydrus init && time ./scripts/hydrus_txt_import.sh test_files/* | tee test_log.txt &&
echo Repeated lines: &&
grep "tag" < test_log.txt | sort | uniq -cd &&
./nothydrus "$@" && tree -a .nothydrus && du -sh .nothydrus

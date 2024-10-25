#!/bin/sh
time ./scripts/hydrus_txt_import.sh test_files_1/* | tee test_log.txt &&
echo Repeated lines: &&
grep "tag" < test_log.txt | sort | uniq -cd

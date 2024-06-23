#!/bin/sh
rm -rd .nothydrus
make && ./nothydrus init && time ./scripts/hydrus_txt_import.sh test_files/* | tee test_log.txt && ./nothydrus "$@" && tree -a .nothydrus && du -sh .nothydrus

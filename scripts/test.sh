#!/bin/sh
rm -rd .nothydrus
make && ./nothydrus init &&
find test_files | ./nothydrus add &&
./nothydrus add_custom_column rating integer 0 0 10 &&
./nothydrus "$@" && tree -a .nothydrus && du -sh .nothydrus

rm -rd .nothydrus
make && ./nothydrus init && time ./nothydrus add test_files/* && ./nothydrus $@ && tree -a .nothydrus && du -sh .nothydrus

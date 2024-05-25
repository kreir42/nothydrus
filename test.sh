rm -rd .nothydrus
make && ./nothydrus init && ./nothydrus $@ && tree -a .nothydrus && du -sh .nothydrus

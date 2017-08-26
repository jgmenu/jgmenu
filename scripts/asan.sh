# This file is designed to be sourced before running jgmenu with ASAN

export ASAN_OPTIONS=detect_leaks=1
export LSAN_OPTIONS=suppressions=$PWD/scripts/lsan.supp

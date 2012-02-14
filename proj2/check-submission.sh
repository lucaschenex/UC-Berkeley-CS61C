#!/bin/bash -e

LOGIN=$USER
TMP=check_submission_tmp

error() {
    echo -e "\e[01;31mERROR: $@\e[00m"
    exit 1
}

ok() {
    echo -e "\e[01;32mOK: $@\e[00m"
}

rm -rf "$TMP"
git clone git@github.com:ucberkeley-cs61c/$LOGIN.git "$TMP" || error "Could not clone your git repo."

cd "$TMP"
cd proj2 || error "You haven't pushed the proj2 directory to github."

git checkout proj2-1 || error "You haven't tagged any commit proj2-1"

make disasmtest &&
    ok "Your proj2-1 submission looks fine to me." ||
    error "Your tagged commit does not pass the sanity check for proj2-1"

rm -rf "$TMP"

exit 0

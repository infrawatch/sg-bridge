#!/bin/env bash
# purpose: runt unit test suite

set -ex

# enable required repo(s)
dnf install -y centos-release-opstools tree
dnf install make gcc qpid-proton-c-devel annobin-annocheck gcc-plugin-annobin rpm-build -y

make tests
./tests

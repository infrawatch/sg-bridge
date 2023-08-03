#!/usr/bin/env bash
set -ex

dnf install make gcc qpid-proton-c-devel annobin-annocheck gcc-plugin-annobin rpm-build -y
make
annocheck --verbose --verbose bridge

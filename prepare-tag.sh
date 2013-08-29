#!/bin/sh

set -e

version_file="cmake/modules/Version.cmake"

while getopts "M:m:p:" flag
do
    case $flag in
    M) major="$OPTARG" ;;
    m) minor="$OPTARG" ;;
    p) patch="$OPTARG" ;;
    esac
done

test -z "$major" && echo "No major given" && exit 1

version="$major"
> "$version_file"

test -n "$minor" && version="$version.$minor" &&
test -n "$patch" && version="$version.$patch"

git tag "v$version"

echo "SET(CPACK_PACKAGE_VERSION_MAJOR \"$major\")" >> "$version_file"
echo "SET(CPACK_PACKAGE_VERSION_MINOR \"$minor\")" >> "$version_file"
echo "SET(CPACK_PACKAGE_VERSION_PATCH \"$patch\")" >> "$version_file"
echo "SET(CPACK_PACKAGE_VERSION \"$version\")" >> "$version_file"

# vim: ft=sh:tabstop=4:et

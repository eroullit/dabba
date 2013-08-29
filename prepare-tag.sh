#!/bin/sh

set -e

version_file="cmake/modules/Version.cmake"
shlibs_file="debian/shlibs"

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
> "$shlibs_file"

test -n "$minor" && version="$version.$minor" &&
test -n "$patch" && version="$version.$patch"

echo "SET(CPACK_PACKAGE_VERSION_MAJOR \"$major\")" >> "$version_file"
echo "SET(CPACK_PACKAGE_VERSION_MINOR \"$minor\")" >> "$version_file"
echo "SET(CPACK_PACKAGE_VERSION_PATCH \"$patch\")" >> "$version_file"
echo "SET(CPACK_PACKAGE_VERSION \"$version\")" >> "$version_file"

echo "libdabba $version" >> "$shlibs_file"
echo "libdabba-rpc $version" >> "$shlibs_file"

git commit -a -s -m "release: tagging v$version"
git tag "v$version"

# vim: ft=sh:tabstop=4:et

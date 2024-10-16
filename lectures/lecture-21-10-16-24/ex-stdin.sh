#!/bin/sh

set -x

while read line; do
  echo "$line"
done < "${1:-/dev/stdin}"

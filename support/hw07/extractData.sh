#!/bin/sh

# CSV from URLhaus contains 9 lines of comments
#  Ninth line is the CSV identification information

# Example CSV comment lines from URLhaus
################################################################
# abuse.ch URLhaus Database Dump (CSV)                         #
# Last updated: 2024-10-27 14:19:30 (UTC)                      #
#                                                              #
# Terms Of Use: https://urlhaus.abuse.ch/api/                  #
# For questions please contact urlhaus [at] abuse.ch           #
################################################################
#
# id,dateadded,url,url_status,last_online,threat,tags,urlhaus_link,reporter

set -x

if [ $# -ne 1 ]; then
    echo "Usage: $0 <URLhaus CSV file>"
    exit 1
fi

# Extract the relevant files from the CSV

head -n 19 $1 > badsite-10.csv
head -n 109 $1 > badsite-100.csv
head -n 259 $1 > badsite-250.csv
sed 1,9d $1 | head -n 10 > badsite-nocmnt-10.csv
sed 1,8d $1 | head -n 10 > badsite-header-10.csv

exit 0

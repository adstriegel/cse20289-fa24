#!/bin/sh
# Check if host is down

set -x

if [ $# -eq 0 ]; then    
    echo "Usage: $(basename $0) host..."    
    exit 1
fi

EXIT=0
for host in $@; do    
    echo -n '$host is '  
    if ping -c 3 $host -W 50 > /dev/null 2>&1; then        
        echo "UP!"    
    else        
        echo "DOWN!"
        EXIT=1    
    fi
done

exit $EXIT

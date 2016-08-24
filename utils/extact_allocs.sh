cat mac.log | grep " alloc " | awk '{print $6}' | sed 's/,/;/g'

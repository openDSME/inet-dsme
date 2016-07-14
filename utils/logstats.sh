cat $1 | sed -rn 's/.*[0-9]+\: ([^ ]*).*/\1/p' | sort | uniq -c | sort -k2nr | awk '{printf("%s\t%s\n",$1,$2)}END{print}' | head -n -1 | sort -n

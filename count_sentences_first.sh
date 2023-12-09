#!/bin/bash

# Check if exactly 1 argument is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: ./$0 <character>"
    exit 1
fi

if ! [[ "$1" =~ ^[[:alnum:]]$ ]]; then
    echo "Argument must be a single alphanumeric character!"
    exit 1
fi

# Count correct sentences
sentence_count=0
while IFS= read -r -p "Enter sentence (Ctrl+D to end): " line; do
    pattern_line=$(echo "$line" | grep -E "^[A-Z][a-zA-Z0-9 ,']*[\.\!\?]$" | grep -E -v ", ?si" | grep "$1")
    if [ -n "$pattern_line" ]; then
        ((sentence_count++))
    fi
done

# Output result
echo "$sentence_count"

exit 0

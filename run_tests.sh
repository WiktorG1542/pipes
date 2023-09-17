#!/bin/bash

# Check if an argument is provided
if [ $# -ne 1 ]; then
  echo "Usage: $0 <directory>"
  exit 1
fi

directory="$1"
sumOfAllTimes=0
count=0

# Check if the provided directory exists
if [ ! -d "$directory" ]; then
  echo "Error: Directory '$directory' not found."
  exit 1
fi

# Iterate over files in the directory
for file in "$directory"/*; do
  if [ -f "$file" ]; then
    # Run the game and capture the output
    output=$(./game TEST "$file" 2>&1)

    # Check if the game ran successfully
    if [ $? -eq 0 ]; then
      echo -e "successfully ran $file in $output seconds."
      sumOfAllTimes=$((sumOfAllTimes + output))
      count=$((count + 1))
    else
      echo "Error: Failed to run game for file '$file'."
    fi
  fi
done

# Calculate the average time
if [ $count -gt 0 ]; then
  averageTime=$((sumOfAllTimes / count))
  echo "Ran $count number of tests."
  echo "Average time per test: $averageTime"
else
  echo "No valid tests were executed."
fi

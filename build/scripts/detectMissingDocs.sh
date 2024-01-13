#!/bin/bash
# Run from /code

# Generate the list of expected topic names
grep -E --only-matching "\"[A-Z][A-Za-z0-9]+\"" src/compiler/BuiltInProcedureList.cpp | sed 's/"//g' | sort | uniq > /tmp/expected_topics.txt

# Generate the list of existing topics
ls doc/help/procedures/ | sed 's/\.txt//g' | sort | uniq > /tmp/existing_topics.txt

# Compare the files to find the missing topics
# -3 suppresses lines that appear in both files, -2 suppresses lines that appear only in the second file (existing topics)
comm -23 /tmp/expected_topics.txt /tmp/existing_topics.txt > /tmp/missing_topics.txt

# Display the missing topics if any
echo "Missing Topics:"
cat /tmp/missing_topics.txt

# Clean up (optional)
rm /tmp/expected_topics.txt /tmp/existing_topics.txt /tmp/missing_topics.txt

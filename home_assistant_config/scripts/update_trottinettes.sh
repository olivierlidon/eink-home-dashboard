#!/bin/bash

# Number of top scooters to track
TOP_N=5

# API endpoint
URL="https://gbfs.api.ridedott.com/public/v2/lyon/free_bike_status.json"

# Bounding box coordinates (lat, lon)
TOP_LEFT_LAT=45.76811428558231
TOP_LEFT_LON=4.856690919710467
BOTTOM_RIGHT_LAT=45.76762793009454
BOTTOM_RIGHT_LON=4.857606657786966

# Temporary files
FILTERED_JSON="/tmp/dott_filtered.json"
TOP5_JSON="/tmp/top5_scooters.json"

# Fetch data
curl -s "$URL" | jq --argjson latMax "$TOP_LEFT_LAT" \
    --argjson latMin "$BOTTOM_RIGHT_LAT" \
    --argjson lonMin "$TOP_LEFT_LON" \
    --argjson lonMax "$BOTTOM_RIGHT_LON" \
    '.data.bikes | map(select(
        (.lat | tonumber) <= $latMax and
        (.lat | tonumber) >= $latMin and
        (.lon | tonumber) >= $lonMin and
        (.lon | tonumber) <= $lonMax and
        (.current_range_meters != null)
    ))' > "$FILTERED_JSON"

# Count scooters number
jq 'length' "$FILTERED_JSON" > "/config/script_data/trottinettes_count.txt"

# Sort and take top 5
jq --argjson n "$TOP_N" 'sort_by(.current_range_meters) | reverse | .[:$n]' "$FILTERED_JSON" > "$TOP5_JSON"

# Create 5 sensor files, filling with 0 if not enough scooters
for ((i=0; i<TOP_N; i++)); do
    VALUE=$(jq -r --argjson index "$i" 'if .[$index] then .[$index].current_range_meters else 0 end' "$TOP5_JSON")
    echo "$VALUE" > "/config/script_data/best_trottinette_$((i + 1)).txt"
done

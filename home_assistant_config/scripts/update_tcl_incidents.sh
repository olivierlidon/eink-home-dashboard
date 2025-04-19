#!/bin/bash

SECRETS_FILE="secrets.yaml"

USERNAME=$(yq e '.grandlyondata_username' "$SECRETS_FILE")
PASSWORD=$(yq e '.grandlyondata_password' "$SECRETS_FILE")

# Auth
AUTH="${USERNAME}:${PASSWORD}"

# API URL
URL="https://data.grandlyon.com/fr/datapusher/ws/rdata/tcl_sytral.tclalertetrafic_2/all.json?maxfeatures=-1&start=1&filename=alertes-trafic-reseau-transports-commun-lyonnais-v2"

# Output files
INCIDENTS_FILE="/config/script_data/tcl_incidents.txt"
MESSAGES_FILE="/config/script_data/tcl_incidents_messages.txt"

# Lines of interest
TARGET_LINES='["A", "B", "C", "T1", "T2"]'

# Current timestamp
NOW=$(date +"%Y-%m-%d %H:%M:%S")

# Fetch and filter data
curl -s -u "$AUTH" "$URL" | jq --arg now "$NOW" \
    --argjson lines "$TARGET_LINES" '
    .values[]
    | select(.niveauseverite <= 20)
    | select(.ligne_cli as $l | $lines | index($l))
    | select(.debut <= $now and .fin >= $now)
' > /tmp/incidents_filtered.json

# Generate nicely formatted incident summary, ex: "Incident A, T1, T2"
INCIDENT_LIST=$(jq -r '.ligne_cli' /tmp/incidents_filtered.json | sort -u | awk '{a[NR]=$0} END{for(i=1;i<=NR;i++) printf "%s%s", a[i], (i<NR?", ": "\n")}')
echo "Incident $INCIDENT_LIST" > "$INCIDENTS_FILE"

# Generate messages summary
jq -r '.ligne_cli as $l | "\($l): \(.message | gsub("^(\\s|\u00a0)+|(\\s|\u00a0)+$"; ""))"' /tmp/incidents_filtered.json > "$MESSAGES_FILE"

MESSAGE=$(cat /config/script_data/tcl_incidents_messages.txt | jq -Rs .)
echo "{\"message\": $MESSAGE}" > /config/script_data/tcl_incidents_messages.json

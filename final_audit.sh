#!/bin/bash

# Fickle vnums
declare -A fickle
for v in 65301 65302 65305 65307 65316 65326 65335 65346 65347 65348 65349 65350 65351 65352 65353 65354 65398 65399 65310 65311 65312 65313 65318 65319 65320 65321 65322 65323 65324 65327 65328 65329 65330 65331 65332 65355 65357 65358 65359 65360 65363 65372 65376 65377 65378 65379 65391 65392 65393 65397; do
    fickle[$v]=1
done

# C handlers
declare -A handlers
for v in 65304 65306 65308 65309 65315 65333 65336 65356; do
    handlers[$v]=1
done

# Active types (trigger gain behavior)
declare -A active_types
for t in 3 4 5 9 15 16 17 19; do
    active_types[$t]=1
done

# Get item types
declare -A types
while IFS=' ' read -r vnum type; do
    types[$vnum]=$type
done < <(awk '
/^#654/ {
    vnum = substr($1, 2)
    card_vnum = vnum - 100
    if (card_vnum >= 65300 && card_vnum <= 65399) {
        for (i = 0; i < 5; i++) getline
        type = $1
        if (type != "") printf "%d %s\n", card_vnum, type
    }
}' lib/world/obj/654.obj)

echo "=== GAPS (NO GAIN COVERAGE) ==="
gaps=0
for v in {65300..65399}; do
    in_fickle=${fickle[$v]:-0}
    in_handler=${handlers[$v]:-0}
    
    type=${types[$v]:-""}
    has_active=0
    if [ ! -z "$type" ] && [ ${active_types[$type]:-0} -eq 1 ]; then
        has_active=1
    fi
    
    covered=$((in_fickle + in_handler + has_active))
    if [ $covered -eq 0 ]; then
        type=${types[$v]:-"NO_ENTRY"}
        gaps=$((gaps + 1))
        printf "%d -> type %s\n" $v "$type"
    fi
done

echo ""
echo "Total gaps: $gaps"

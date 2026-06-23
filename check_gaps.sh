#!/bin/bash

# Fickle vnums
fickle="65301 65302 65305 65307 65316 65326 65335 65346 65347 65348 65349 65350 65351 65352 65353 65354 65398 65399 65310 65311 65312 65313 65318 65319 65320 65321 65322 65323 65324 65327 65328 65329 65330 65331 65332 65355 65357 65358 65359 65360 65363 65372 65376 65377 65378 65379 65391 65392 65393 65397"

# C handlers
handlers="65304 65306 65308 65309 65315 65333 65336 65356"

# Active types
active_types="1 2 3 4 5 9 15 16 19 25"

# Get item types
declare -A types
while IFS=' ' read -r vnum type; do
    types[$vnum]=$type
done < <(awk '
/^#[0-9]+$/ {
    vnum = substr($1, 2)
    card_vnum = vnum - 100
    if (card_vnum >= 65300 && card_vnum <= 65399) {
        getline; getline; getline; getline
        type = substr($1, 1, 1)
        printf "%d %s\n", card_vnum, type
    }
}' lib/world/obj/654.obj)

echo "GAPS (no fickle, no C handler, no active type):"
for v in {65300..65399}; do
    # Check if in fickle
    in_fickle=0
    for f in $fickle; do
        if [ "$v" = "$f" ]; then
            in_fickle=1
            break
        fi
    done
    
    # Check if C handler
    in_handler=0
    for h in $handlers; do
        if [ "$v" = "$h" ]; then
            in_handler=1
            break
        fi
    done
    
    # Check if active type
    has_active=0
    type=${types[$v]}
    if [ ! -z "$type" ]; then
        for at in $active_types; do
            if [ "$type" = "$at" ]; then
                has_active=1
                break
            fi
        done
    fi
    
    # If not covered by any of three sources
    if [ $in_fickle -eq 0 ] && [ $in_handler -eq 0 ] && [ $has_active -eq 0 ]; then
        type=${types[$v]}
        if [ -z "$type" ]; then
            type="NO_ENTRY"
        fi
        echo "  $v (type $type)"
    fi
done

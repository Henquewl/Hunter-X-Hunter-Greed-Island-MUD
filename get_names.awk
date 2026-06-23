BEGIN {
    cards[65300] = "Ruler's Blessing"
    cards[65317] = "Angel's Breath"
    cards[65325] = "Risky Dice"
    cards[65337] = "Fledgling Athlete"
    cards[65338] = "Fledgling Artist"
    cards[65339] = "Fledgling Politician"
    cards[65340] = "Fledgling Musician"
    cards[65341] = "Fledgling Pilot"
    cards[65342] = "Fledgling Novelist"
    cards[65343] = "Fledgling Gambler"
    cards[65344] = "Fledgling Actor"
    cards[65345] = "Fledgling CEO"
    cards[65361] = "Vending Checkup"
    cards[65371] = "Scientists' Pheromones"
    cards[65373] = "Night Jade"
    cards[65374] = "Sage Aquamarine"
    cards[65375] = "Wild Luck Alexandrite"
    cards[65380] = "Levitation Stone"
    cards[65381] = "Blue Planet"
    cards[65384] = "Paladin Necklace"
    cards[65386] = "Quiver of Frustration"
    cards[65396] = "Clairvoyant Snake"
    
    types[11] = "ITEM_WORN"
    types[12] = "ITEM_OTHER"
    types[13] = "ITEM_TRASH"
    types[8] = "ITEM_TREASURE"
    types[""] = "NO_ENTRY"
}

/^#654/ {
    vnum = substr($1, 2)
    card_vnum = vnum - 100
    if (card_vnum in cards) {
        for (i = 0; i < 5; i++) getline
        type_val = $1
        printf "%d %-35s <- type %s (%s)\n", card_vnum, cards[card_vnum], type_val, types[type_val]
    }
}

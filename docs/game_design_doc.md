# Game Design Document

## Player Goals

### Ultimate Goal
* To win a game of Ultimate Naughts-and-Crosses.

### Intermediate Goal
* To win individual games of Naughts-and-Crosses.

### Short-term Goal
* To place their naught or cross in the desired location on the board.
* This is done by travelling across the board in a spaceship that moves according to a simplified version of classical mechanics in space (i.e. no air resistance).
* This is further complicated by several hazards that are capable of hindering the player's ability to place their naught/cross in their desired position.

## Main Game Loop

* Players spawn on field, a subset of the potential environmental hazards are selected to appear on the field for this round.

* The 'orb' spawns somewhere on the outside of the board.

* Players attempt to reach the orb whilst avoiding environmental hazards and attacks from the other player.

* Once one of the players comes into physical contact with the orb, that player is now considered to be 'priming the charge'. This means that whilst that player is in possesion of the orb, it will slowly turn into that player's affiliated symbol (naught or cross).

* The player with the charge will now have to avoid being stunned or killed whilst the orb is priming, else they will lose the orb and a new orb will spawn on the map.

* Once the orb is charged, the player will have the option to place it on the currently active naughts-and-crosses grid (as determined by the poisiton of the previously placed naught/cross) by flying over the desired square and [doing some interaction].

* If successful, the player's charge will be placed on the board and the round ends. The next active grid will be selected based on the grid location that the charge was just placed in, and the next round begins. Repeat loop.

* If at any point a player is hit by a hazard or the other player's bullets, then that player will be stunned and is temporarily be locked of their controls, meaning the ship will effectively 'spin out'.

* If at any point a player is killed (by moving out of the play area), then the round will end and the surviving player may place their charge on any of the available squares on the active grid.

## Player Abilities

* Can turn left and right.
* Can apply thrust in the forward direction.
* Can fire a Light Action Sub-Entropic Round (L.A.S.E.R)
* Can pick up the orb by flying directly over it.
* Can prime the orb whilst in posession of it (passive ability).
* Can place the primed orb on an available square in the active grid with the [interaction key].

## Environmental Hazards

* Asteroids.
* Solar Wind(?).
* Randomly appearing black hole(s).
* Pirates that whizz through the map.
* Nebulas or something I don't know.

## Environmental Benefits

* Weapon pickups.
* Weapon and ship powerups.

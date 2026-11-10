# Commodore 64 Homebrew Arcade Games

Source code and assets for a collection of arcade-style C64 soccer games. All projects are human-crafted passion projects focused on original gameplay, custom PETSCII graphics, and SID sound, with code structure optimized using **AI assistance**.

Tested on real Commodore 64 hardware, the VICE emulator, and MiSTer FPGA.

All these games are available to download at https://emmonks.itch.io
---

## 1. 2GOAL Series

Fast-paced, showbol-inspired arcade soccer featuring intense 1v1 action, walls, and golden-score mechanics.

### Variants & Game Modes:

* **1. CLUB WORLD CUP**
  * Compete against the 16 top-ranked club teams from around the globe in a high-stakes club championship.

* **2. WORLD CUP**
  * Take the international stage! Choose from the top 16 national teams based on official rankings.

* **3. C64 LEAGUE**
  * Compete in a full league season with teams inspired by classic C64 games. This mode supports up to 4 human players distributed across different teams.
  * **Multiplayer Configurations:**
    * 2 Players: P1 (Human) + CPU vs. P3 (Human) + CPU, or P1 + P2 (Human) on the same team.
    * 3 Players: P1 (Human) + CPU vs. P3 + P4 (Human).
    * 4 Players: P1 + P2 (Human) vs. P3 + P4 (Human).
  * **Structure:** Choose a league size from 4 to 12 teams and customize the number of rounds to set the season length.
  * **Points:** Win = 3 pts | Draw = 1 pt | Loss = 0 pts.
  * **League Table:** Track your stats via P (Points), W (Wins), GF (Goals For), and GA (Goals Against). Human-controlled teams are marked with an asterisk (*).

* **4. CHAMPIONS LEAGUE**
  * A classic tournament format featuring group stages followed by intense knockout rounds, starring legendary European clubs that dominated the 1980s.

* **5. FRIENDLY/TOURNAMENT**
  * A bracket-style mode with progressive difficulty. Defeat increasingly tougher opponents to lift the champion's trophy.
  * A quick, action-packed match between Team A (P1 + P2/CPU) and Team B (P3 + P4/CPU).
  * **Duration:** 45 seconds (1st and 2nd half) or first to score 5 goals.
  * **Teams:** 20 available classic-inspired squads, national selections, and worldwide club teams (including Giana United, Turrican FC, Wizball City, Real Arkanoid, and many more).

* **7. TRAINING/CHALLENGE**
  * The perfect mode to master gameplay mechanics. It also doubles as a unique "free-for-all" competition where every player fights individually and can score on any goalkeeper.
  * **Bonus Features:** Player 2 has the exclusive option to play manually as the goalkeeper. Includes a built-in utility tool to test your 4-joystick adapter setup.
  * Progress through 18 stages with increasing difficulty levels.

---
<!--
## 2. 4GOAL Series

The multi-net soccer foundation balancing tactical positioning, goal-zone defense, and arcade speed.

### Variants & Game Modes:

* **4GOAL (Classic)**
  * The original multi-net arcade soccer foundation.
* **4GOAL GK**
  * Specialized goalkeeper-focused gameplay variant testing reflexes and penalty positioning.
* **4GOAL 4P**
  * 4-player simultaneous arcade action utilizing adapter hardware extensions.
* **4GOAL CHALLENGE**
  * Progressive difficulty matches and tournament survival challenges.
* **4GOAL LEAGUE**
  * Full league structure tracking standings, points, and multi-match campaigns.
* **4GOAL WORLDCUP**
  * International bracket tournament mode simulating global competition.

---
-->
### 3. Assets and Tools

* **4 joysticks driver (Protovision)**
  * **ptvjoy.s** - Assembly code to enable 4 joysticks compatible with Protovision adapter
* **4 joysticks Tester (Protovision)**
  * **teste4p.c** - Simple tester to 4 joysticks compatible with Protovision adapter
* **Menu return (Assembly)**
  * **load_menu.s** - Function to returno from game menu

## Prerequisites

To compile these projects from source, you need the **cc65** C compiler suite installed on your system (ensuring `cc65`, `ca65`, and `ld65` are in your system PATH).

---

## Compilation & Build Instructions

Navigate to the respective game directory and use the standard `cc65` build commands to generate the C64 executable (`.prg`):

### 1. 2GOAL CLUB WORLD CUP

```bash
cl65 -t c64 -Oir -o 2GOALWT.prg 2GOAL_WT.c ptvjoy.s 
```

### 2. 2GOAL  WORLD CUP

```bash
cl65 -t c64 -Oir -o 2GOALWC.prg 2GOAL_WC.c ptvjoy.s 
```

### 3. 2GOAL C64 LEAGUE

```bash
cl65 -t c64 -Oir -o 2GOALLE.prg 2GOAL_LE.c ptvjoy.s 
```

### 4. 2GOAL CHAMPIONS LEAGUE

```bash
cl65 -t c64 -Oir -o 2GOALCL.prg 2GOAL_CL.c ptvjoy.s 
```

### 5. 2GOAL  FRIENDLY/TOURNAMENT

```bash
cl65 -t c64 -Oir -o 2GOALFR.prg 2GOAL_FR.c ptvjoy.s 
```
### 6. 2GOAL TRAINING/CHALLENGE

```bash
cl65 -t c64 -Oir -o 2GOALTC.prg 2GOAL_TC.c ptvjoy.s 
```


### 7. 4P Tester

```bash
cl65 -t c64 -Oir teste4p.c ptvjoy.s -o 4PJOYT.prg
```
### 8. 2GOAL INTRO

```bash
cl65 -t c64 -Oir -o 2GOALv2_INTRO.prg 2GOAL_INTRO.c  assets.s assets2d.s

```
### 9. 2GOAL MENU

```bash
cl65 -t c64 -Oir -o 2GOALMENU.prg 2GOAL_MENU.c
```

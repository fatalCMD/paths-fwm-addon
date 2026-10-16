# Paths - Follower Ability Skilltree

An add-on for Fight With Me: a perk tree for each follower role. Every level a companion earns gives them a perk point to spend on the path of the role they play: new abilities, ranks in the ones they know, branches that add to them, and traits that are always on. They carry two abilities into a fight, and you choose which.

It lives inside Fight With Me's battle plan: no extra menu, no ESP.

## Requirements

- Fight With Me 0.3.0 or later
- SKSE64 and Address Library for SKSE Plugins
- SKSE Menu Framework 3 (already needed for the battle plan)

Skyrim SE 1.5.97 and AE are both supported, like Fight With Me itself.

## How it works

- **Points.** One for every level past 10: the first at level 11, forty at level 50. Every companion has their own, from the highest level they themselves have reached, so a companion who was with you before you installed this, or one you meet at level 30, already has every point their level earned. One companion's points are never another's.
- **Paths.** Each role has its own: the Blade (Fighter), the Shield (Tank), the Mender (Healer), the Hammer (Bruiser), the Arrow (Archer) and the Storm (Mage). A companion can spend their points on any of their paths, but only the path of the role they are playing counts in a fight.
- **Abilities.** Every path has four. The first two are known from the start at rank I, weak but useful. The third opens once five points are spent on the path, the fourth once ten are (on the Storm, the fourth is Magnus; see below).
- **Ranks and branches.** Every ability ranks up to III, replacing the level milestones Fight With Me used before. Each ability has two branches that add something to it, each up to three ranks; a branch can never outrank its ability.
- **Traits.** Three per path, up to three ranks each, always on while they fight.
- **Path bonus.** Every point spent on a path also makes a companion of that role 1% better at what the role is for, shown beside the path's name. Fighter: attack speed. Tank: less damage from every blow, arrow and harmful spell, 1% less per point (75% at most). Healer: all the healing they do, from abilities, the spells they cast and the potions they drink. Bruiser: weapon damage. Archer: the chance an arrow they shoot is not used up (90% at most). Mage: magicka cost of every spell, 1% less per point (75% off at most). Only the path of the role they play now counts. The rate per point is set in Fight With Me's INI, `[PathBonus]`. Once every node of a path is mastered, leftover points can go straight into its bonus (the card's INTO THE BONUS button, or A on a pad), until the bonus is at its highest; the − beside the bonus takes one back out, and starting the path over returns them all.
- **Two carried.** Each path carries two abilities into a fight. Swap them out of a fight; each has its own AUTO switch for using it unasked.
- **Giving points back.** Right-click a node (or Y on a pad) to give back a rank, or start a whole path over, free, out of a fight. Can be switched off in the INI.

## The paths

| Path | Abilities | Traits |
|---|---|---|
| Blade (Fighter) | Disarm, Second Wind, Riposte, Cleave | Flanker, Endurance, Opportunist |
| Shield (Tank) | Taunt, Bodyguard, Challenge, Stand Fast | Iron Hide, Shield Master, Last Stand |
| Mender (Healer) | Heal, Rejuvenate, Ward, Renewal | Triage, Serenity, Guardian Spirit |
| Hammer (Bruiser) | War cry, Berserk, Sunder, Earthshaker | Heavy Hands, Momentum, Executioner |
| Arrow (Archer) | Volley, Deadeye, Pinning Shot, Arrow Rain | Fletcher, Skirmisher, Sharpshooter |
| Storm (Mage) | Arcane Surge, Ward, Elemental Burst, and Magnus (capstone) | Arcane Flow, Spell Shield, Unravel |

**Magnus** sits at the top of the Storm on its own: it opens only once every other node on the path is mastered, every rank of it (30 points), costs 5 points to learn and 3 for each rank after. For a while their magicka never runs dry; rank II makes their destruction spells strike harder, rank III adds resistance to magic.

Every node says what each of its ranks does in the battle plan.

## Using it

Open the battle plan. On a companion's card, the abilities panel shows the two they carry and a **Path** tile with any points waiting. Choose the tile, or one of the two abilities out of a fight, to open their path.

The first time a path opens, a short guide walks through the page a part at a time. It is always under the **?** in the page's header, or F1 (View on a pad).

| | Mouse and keyboard | Pad |
|---|---|---|
| Choose a node | Click, arrows | D-pad |
| Learn a rank | Enter, or click the chosen node again | A |
| Carry in slot 1 / 2 | 1 / 2, or click a slot | X |
| Give back a rank | Right-click, Backspace | Y |
| Another path | Q / E, or the tabs | LT / RT |
| Another companion | Tab | LB / RB |
| The guide | F1, or the ? | View |
| Close | Esc | B |

### Unlimited points

The **Unlimited** button (∞) in the path page's header, beside the guide's **?**, lets that companion buy every rank on every path for free. While it's on, their tree carries a picture and the word CHEATER in the background, and each ability they use has a chance of a chicken's cluck.

Pressing it again (out of a fight) puts them back exactly where they were before: every rank bought since is given back, and an ability they no longer know is swapped for a default of their role that they aren't already carrying. Points earned by levelling in the meantime are kept. The switch is saved with the game, per companion.

## Settings

Paths follows Fight With Me's **Battle plan > Interface > Theme** choice in the Mod Control
Panel: **Oxblood** keeps the original red (the default), **Vanilla** uses charcoal, silver
and pale starlight, and **Nordic** adds engraved stone frames and beveled metal emblems.
Every theme keeps the existing tree layout, windows, navigation and controls. One choice
updates both menus; there is no separate Paths theme.

With Paths installed, Fight With Me's **Tanks** and **Healers** settings pages are replaced by
a cover, *Your Followers Are Guided by Their Path*, and can no longer be changed: a taunt's
hold, cooldown and bracing armor, a heal's amount and cooldown, and how long Rejuvenate lasts
come from the ranks on each path, and everything else on those pages stays at Fight With
Me's defaults. Anything you had changed there is kept in `FightWithMe_user.ini` and comes
back if Paths is removed.

`SKSE/Plugins/FightWithMePaths.ini`:

- `iPointsAboveLevel` (10): one point for every level past this one. 1 gives a point for every level from 2.
- `iBonusPoints` (0): extra points for every companion.
- `bAllowRespec` (1): ranks can be given back.
- `bAllowUnlimitedPoints` (1): the Give unlimited points button. 0 removes it, except for a companion already on unlimited points, who keeps the button until you switch them back.

## Removing it

Fight With Me goes back to its own abilities, growing with level. Nothing is left on your companions: every lift an ability gives is temporary and is taken off again after a load.

## Building from source

The plugin is built inside a [Fight With Me](https://github.com/fatalCMD/Fight-With-Me) checkout, from the same trees (`src/Paths.cpp`), so the two DLLs always agree on them. Clone Fight With Me with its submodules, clone this repository into its `paths/` folder, configure with the `skyrim` CMake preset and build the `FightWithMePaths` target, or run `paths/tools/BuildPathsPackage.ps1`, which also runs both test suites.

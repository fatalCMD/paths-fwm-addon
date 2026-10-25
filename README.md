# Path of the Companions 1.0

A perk tree for each follower role. Every level a companion earns gives them a perk point to spend on their paths: new abilities, ranks in the ones they know, branches that add to them, and traits that are always on. They carry two abilities into a fight, and you choose which.

It works on its own, and it works with [Fight With Me](https://www.nexusmods.com/skyrimspecialedition/mods/191283):

Either way Paths runs the abilities, the unique traits companions bring from their own mods, and the levels they earn by fighting, and its panel at the bottom right of the screen shows the companion, their level and perk points, the two abilities they carry, and the skill tree.

- **On its own**, companions use the two they carry when a fight calls for it; nothing else about how they fight changes. The path they follow starts as the one their gear suits, and you change it on their tree.
- **With Fight With Me**, the role you give a companion in its battle plan is the path that counts, the enemy you send them after is the one their abilities go for, and Fight With Me carries out what the abilities ask of a fight. Fight With Me itself changes only whom they fight and where they stand.

No ESP.

## Requirements

- SKSE64 and Address Library for SKSE Plugins
- SKSE Menu Framework 3, for the panel and the trees
- **POTC Effects**, the art the abilities use (meshes and textures only; no plugin). Fight With Me users already have it.
- Optional: Fight With Me 0.5.0, which provides battle orders and role-aware loadout pools. Update the two plugins together.

Skyrim SE 1.5.97 and AE are both supported, like Fight With Me itself.

### Updating after the plugin split

Update both DLLs and their shipped INIs together. Keep `FightWithMe_user.ini` and
`FightWithMePaths_user.ini`: these hold your personal settings. Paths now opens its
own panel with **Numpad 9 / LB + D-pad right**, even when Fight With Me is installed.
The battle plan stays on **Numpad 7 / LB + D-pad down**. Untouched older defaults
that collide are repaired at runtime; explicitly customized bindings are kept.

The optional matched-core bundle includes both DLLs; install the current base package
first so its default settings are current too. Effects **1.0.21 or newer** is required
for the latest ability artwork. Paths includes its own follower-trait definitions.

## Party damage balance

Under **Settings > Party balance**, reduce follower damage as the active party grows.
The defaults put four followers at 55% damage each against one enemy, 65% against
two, and 85% against four or more. One follower deals full damage; the default
minimum for larger parties is 50%. You can adjust the strength or switch it off.
This covers normal weapon/spell damage (including other mods) and POTC abilities.
Player damage and healing are unchanged. Direct scripted damage without an attacker
may bypass it; existing spell effects retain their multiplier until they expire.

## Effect update

POTC Effects 1.0.21 accompanies this refreshed 1.0 build. In the Paths settings,
**Look > Compact summons** is on by default for smaller spirit heads, Dragon's
Breath, Azura's Moon and Last Bastion. Turn it off to restore their original size.
Magic circles and Magnus keep their existing size.

Dragon's Breath uses Skyrim's native Fire Breath shout effect, denser fire and frost,
and stronger lightning. Base damage is 20 / 30 / 45 per second. Arrow Rain now fires
24 / 36 / 48 fast arrows through a forward portal over 1.5 seconds; Downpour adds
8 / 16 / 24 arrows. Update both the plugin and the Effects download.

## How it works

- **Points.** One for every level past 10: the first at level 11, forty at level 50. Every companion has their own, from the highest level they themselves have reached, so a companion who was with you before you installed this, or one you meet at level 30, already has every point their level earned. One companion's points are never another's.
- **Paths.** Each role has its own: the Blade (Fighter), the Shield (Tank), the Mender (Healer), the Hammer (Bruiser), the Arrow (Archer) and the Storm (Mage). A companion can spend their points on any of their paths, but only the path of the role they are playing counts in a fight.
- **Abilities.** Every path has four. None are unlocked by default. The first two can be purchased immediately. The third unlocks at follower level 35 and the fourth at level 50, independently of earlier purchases. Magnus also requires every other Mage perk to be unlocked (rank I).
- **Ranks and branches.** Ability ranks I, II and III cost 1, 2 and 3 points respectively (6 total). Branch ranks cost 1 point each. Each ability has two branches that add something to it, each up to three ranks; a branch can never outrank its ability.
- **Traits.** Three per path, costing 1 point per rank. Rank I is available to purchase immediately; rank II requires follower level 25 and rank III level 50. Debug Mode bypasses these gates. Learned traits are always on while they fight.
- **Path bonus.** Every point spent on a path also makes a companion of that role 1% better at what the role is for, shown beside the path's name. Fighter: attack speed. Tank: less damage from every blow, arrow and harmful spell, 1% less per point (75% at most). Healer: all the healing they do, from abilities, the spells they cast and the potions they drink. Bruiser: weapon damage. Archer: stamina recovery in combat (+90% at most); arrows are unlimited before spending any points. Mage: magicka cost of every spell, 1% less per point (75% off at most). Only the path of the role they play now counts. The rate per point is set in `FightWithMePaths.ini`, `[PathBonus]`. Once every node of a path is mastered, leftover points can go straight into its bonus (the card's INTO THE BONUS button, or A on a pad), until the bonus is at its highest; the âˆ’ beside the bonus takes one back out, and starting the path over returns them all.
- **Two carried.** Each path carries two abilities into a fight. Swap them out of a fight; each has its own AUTO switch for using it unasked.
- **Giving points back.** Right-click a node (or Y on a pad) to give back a rank, or start a whole path over, free, out of a fight. Can be switched off in the INI.

## Progression update

Loading a co-save from before this progression update resets all companion paths once, including bonus investments and saved unlimited-point snapshots. Earned levels and point totals are preserved: all earned points become available to spend again. No abilities are carried until purchased. New saves retain purchases normally.

## The paths

| Path | Abilities | Traits |
|---|---|---|
| Blade (Fighter) | Disarm, Second Wind, Riposte, Kynareth's Maw | Flanker, Endurance, Opportunist |
| Shield (Tank) | Taunt, Weak Will, Chaos Down, Last Bastion | Iron Hide, Shield Master, Last Stand |
| Mender (Healer) | Healing Circle, Renewal, Rejuvenate, Azura's Moon | Triage, Serenity, Guardian Spirit |
| Hammer (Bruiser) | War cry, Berserk, Sunder, Earthshaker | Heavy Hands, Momentum, Executioner |
| Arrow (Archer) | Volley, Explosive Arrow, Windstep, Arrow Rain | Steady Breath, Fine Arrows, Spell-Touched |
| Storm (Mage) | Arcane Surge, Elemental Burst, Dragon's Breath, and Magnus (capstone) | Arcane Flow, Spell Shield, Unravel |

**Magnus** sits at the top of the Storm on its own: it opens at follower level 50 once every other nonempty node on the path has at least rank I. Full mastery is not required. Its ranks cost 1/2/3 points, like other abilities. For a while their magicka never runs dry; rank II makes their destruction spells strike harder, rank III adds resistance to magic.

Every node says what each of its ranks does in the battle plan.

**Chaos Down** replaces Challenge in the Shield's third ability slot. It slows enemies within
250 units of the tank by 25/40/55% for 8 seconds, with a 25-second cooldown. Widening Chaos adds
50/100/150 units to the radius; Lingering Chaos adds 2/4/6 seconds. It affects enemies in reach
when activated, with line of sight, and automatically triggers when at least two are nearby.
Repeated casts keep the strongest slow and refresh its duration instead of stacking. Existing
Challenge, Humble and Duel ranks become Chaos Down, Widening Chaos and Lingering Chaos respectively;
equipped slots and spent points are preserved.
Ranks I-II summon the upper half of a giant violet skeleton, leaning its whole body over the circle
and drawing both open hands inward and down. Rank III summons a deep purple draugr wearing the Jagged
Crown, with soft purple mist rising and drawing inward from the circle. Binding runes follow affected enemies for the
slow's duration; Lingering Chaos extends those runes with the slow. Ability effects can be switched
off without changing the gameplay.

## Using it

Press Numpad 9 (LB + D-pad right on a pad; both can be changed; Fight With Me's battle plan keeps Numpad 7). The panel opens at the bottom right on the companion under your crosshair, or the first one in reach: their name and level on the band, the path they follow and their perk points under it, then the two abilities they carry, each with its AUTO switch, and **Skill tree** with any points waiting. Tab, LB / RB or the diamonds under the band move to another companion. Out of a fight, choose Skill tree, or one of the two abilities, to open their tree; in a fight, choosing an ability uses it now. Esc or B closes the panel. The world slows while it is open.

The path a companion follows starts as the one their gear suits (shield: the Shield, two-hander: the Hammer, bow: the Arrow, staves or spells: the Storm or the Mender) and changes on the tree: turn to another path and choose **Follow this path** under the dropdown, out of a fight.

**With Fight With Me:** the panel is the same; the path that counts is the role the battle plan gives them, so there is no Follow this path.

The tree fills the window. Select a perk to open its attached detail panel and shift
the tree left. Click empty space or press Esc / B to close the details and center the
tree again. Point at a rank to preview its effects; use the Learn / Rank Up button
to spend points. The path bonus and its investment controls sit beside the title.

The first time a path opens, a short guide walks through the page a part at a time. It is always under the **?** in the page's header, or F1 (View on a pad).

| | Mouse and keyboard | Pad |
|---|---|---|
| Choose a node | Click, arrows | D-pad |
| Learn a rank | Enter with details open, or click Learn / Rank Up | A with details open |
| Carry in slot 1 / 2 | 1 / 2, or click a slot | X |
| Give back a rank | Right-click, Backspace | Y |
| Another path | Q / E, or the tabs | LT / RT |
| Another companion | Tab | LB / RB |
| The guide | F1, or the ? | View |
| Close details, then the path | Esc | B |

### Debug Mode

The **Debug** button beside the guide's **?** enables testing for that companion. Perk points are unlimited, and all ranks can be learned without level or prerequisite gates. Learn the desired rank, place it in one of the two normal ability slots, and use its icon in the battle menu. Those icons also work outside combat in Debug Mode, without spending magicka or stamina or starting a cooldown. An already active effect finishes before it can be repeated.

Launched abilities aim at a fixed point **10 metres ahead** of the follower; they do not need an enemy. Self effects remain on the follower. Ranged tests copy a bow and arrow form if needed, without adding equipment to their inventory. Sunder demonstrates its armor overlay on the caster without lowering their armor, and Disarm demonstrates its gesture when there is no opponent. The old artwork, label, and chicken sounds have been removed.

Pressing it again (out of a fight) puts them back exactly where they were before: every rank bought since is given back, and an ability they no longer know is swapped for another learned ability, or leaves the slot empty if none remain. Points earned by levelling in the meantime are kept. The switch is saved with the game, per companion.

## Settings

Paths has one page in the Mod Control Panel, **Path of the Companions > Settings**: the perk point rules below, levels earned by fighting, the panel's key and gamepad button, ability effects, notices, tooltips and menu sounds, and theme selection (with or without Fight With Me). Changes apply at once and are saved to `FightWithMePaths_user.ini`, which updates never replace.

The theme: **Oxblood** keeps the original red (the default), **Vanilla** uses charcoal, silver
and pale starlight, and **Nordic** adds engraved stone frames and beveled metal emblems.
Every theme keeps the existing tree layout, windows, navigation and controls. With Fight With
Me, Paths follows its **Battle plan > Interface > Theme** choice; one choice updates both menus.

`SKSE/Plugins/FightWithMePaths.ini`, `[Points]`:

- `iPointsAboveLevel` (10): one point for every level past this one. 1 gives a point for every level from 2.
- `iBonusPoints` (0): extra points for every companion.
- `bAllowRespec` (1): ranks can be given back.
- `bAllowUnlimitedPoints` (1): shows Debug Mode. The legacy key is retained for compatibility. 0 hides the button, except for a companion already in Debug Mode, who keeps it until switched back.

`[Leveling]` sets the levels earned by fighting (below), `[PathBonus]` the bonus per point on each path, `[Panel]` the key (`iKey`, `iKeyModifier`), the pad button (`iGamepadKey`), how fast the world runs while it is open (`fTimeScale`), `bHideHUD` and `fScale`; `[Effects] bEnabled` switches the abilities' look off (not what they do), and `[Interface]` the theme, notices, sounds, tooltips and language. With Fight With Me, the theme is its own.

## Levels earned by fighting

Companions level up by fighting beside you. When a fight is over, each companion who was in it earns XP for every enemy the party brought down, whoever struck last; one who sat it out gets nothing. A level costs what one of yours would, and more the higher they are (`fClimb`). Each brings ten points of health, magicka or stamina by the path they follow, and a point in its skills, up to 100. Nobody passes your level (`bPlayerLevelCap`). Every level past 10 is a perk point. Off (`[Leveling] bEnabled`, or the settings page): everyone goes back to the level their own mod gave them, and what levels gave them is taken off again.

## Unique traits

The companion card shows up to two rows of signature abilities, with extra abilities under

the `+N more` chip. Hover a chip for its effect and trigger, or the rarity label for the count.

Rarity counts distinct unlocked abilities: zero is Common, one Uncommon, two Rare, three Epic,

and four or more Legendary. Locked moves and morale drawbacks never raise rarity.



Detection uses the companion's original plugin, excluding shared overhaul and framework perks.

Script-only effects and quest event handlers need named entries in

`SKSE/Plugins/FightWithMe/Traits/*.ini`, since combat events also run dialogue and utility code.

Entries can name an ability from another plugin; matching names merge into one move. The

bundled notes identify Katana's Shadow Step, Mist Raven and Spectral Warband, and Megara's

Battle Boon and Shadow Cloak. `Name`, `What`, optional `When`, `Kind` and `Hide` are documented

in `config/Traits/Followers.ini`.

## Removing it

Nothing is left on your companions: every lift an ability gives is temporary and is taken off again after a load, and levels earned by fighting go back to what their own mods give them. Fight With Me keeps only its orders.

Paths keeps its records in its own part of the co-save. Installing Fight With Me later keeps every path and rank; the path each companion follows then comes from the role Fight With Me gives them.

## Translating

The trees' text (paths, abilities, ranks, branches and traits) is in `SKSE/Plugins/FightWithMe/Translations/FightWithMePaths_ENGLISH.txt`: the English, a tab, then the English again. Copy it to the game's language name (`FightWithMePaths_FRENCH.txt`), translate the right-hand side and ship it in the same folder; Paths and Fight With Me both read it. Keep every `{}` (a value filled in as the game runs; `{0}`, `{1}` put them in another order) and `\n`. The rest of the panel and the path page is in `FightWithMe_<LANGUAGE>.txt` (shipped beside it), which both read too.

## Building from source

The plugin is built inside a [Fight With Me](https://github.com/fatalCMD/Fight-With-Me) checkout, from the same sources, so the two DLLs always agree on the trees and the abilities: the Paths DLL is built with `FWM_HOST_PATHS` and runs the abilities, traits and levels, and Fight With Me's DLL is built without them. Clone Fight With Me with its submodules, clone this repository into its `paths/` folder, configure with the `skyrim` CMake preset and build the `FightWithMePaths` target, or run `paths/tools/BuildPathsPackage.ps1`, which also runs both test suites.

## Archer ammunition and traits

Archers following the Arrow path have unlimited ammunition without spending perk points. An empty bow quiver receives one basic iron arrow. Crossbows retain their current bolts and need an initial compatible bolt. Volley does not consume ammunition either. The player, enemies, other roles and companions without an active path retain their usual ammunition behavior.

- **Steady Breath** replaces Fletcher: 25/50/75% faster stamina recovery in combat.
- **Fine Arrows** replaces Skirmisher: supplies steel/elven/ebony arrows at ranks I/II/III and equips them for bows. Stronger arrows already in the companion's inventory are preferred. It supplies a single reusable arrow, not growing stacks.
- **Spell-Touched** replaces Sharpshooter: arrows and bolts roll a 10/15/20% chance once as flight begins, choosing fire, frost or lightning. Original textured flame/embers, frost mist/shards or flickering lightning arcs follow that specific projectile; the same element strikes on impact. Each unblocked proc deals 8 health damage before magic and elemental resistance. It does not add an explosion, stagger, slow or resource drain; blocked hits and bow bashes cannot trigger it. The impact uses the mod's elemental effects and Skyrim's hit sound when available.

All three replacements retain the original perk IDs. The progression update below resets earlier investments once. The old extra retreating behavior and flat Sharpshooter damage modifier are removed.


The Arrow path bonus is now +1% stamina recovery in combat per point, capped at +90%, instead of a chance to save arrows. Existing points invested directly in that bonus carry over. The new tuning key is `[PathBonus] fArcherStaminaRecovery`; the obsolete `fArcherArrowsKept` key is ignored.

### Dragon's Breath and War Cry

Elemental Burst now occupies the second Mage ability row. Dragon's Breath occupies the third, with Dragon's Reach (+10/20/30% reach) and Lingering Breath (+0.5/1/1.5 seconds) branches. Mage Ward is removed; the Healer's former Ward is now Azura's Moon. Existing Elemental Burst ranks follow it to its new row, and points previously spent on Mage Ward are freed on load.

Dragon's Breath channels the same equipped element as Elemental Burst. Ranks I/II/III deal 8/11/14 damage per second for 2.4/2.8/3.2 seconds, over 600/850/1100 units before branches (38-second cooldown). It affects enemies inside the growing stream, respecting magic and elemental resistance and cover. Frost also drains stamina; lightning drains half as much magicka. It never damages party members. Ranks I-II use a blue spectral dragon; rank III uses red Alduin. The dragon draws back, lifts its skull as the lower jaw opens, and settles into the blast. A rotating spell seal forms behind it; the mouth-anchored stream and matching elemental sound persist throughout the channel, then fade. Casting requires at least two metres from any nearby enemy and a clear firing line; automatic use prefers more room for the head. The caster remains planted during the summon, channel and fade, with animation and gravity preserved, and is released immediately on interruption or cleanup.

War Cry now uses a golden sabercat head at ranks I-II and a larger crimson sabercat at III. All ranks have additional clearance for their entire roar animation.

Dragon's Breath's beam now travels level in front of the follower. Its diameter is 180/250/360 units before reach bonuses, with a short widening at the mouth and a broad, constant body. The head sits directly ahead with its animated open jaw just above the caster's ground plane; rank III Alduin is larger. Lightning strands re-form independently, and frost particles have radial transparency instead of hard sprite edges.

Dragonï¿½s Breath streams use round textured geometry instead of crossed flat sheets. Rank III has a blue-hot fire core, dense pale frost with moving ice shards, or connected lightning bolts with occasional forks traced down onto terrain. These forks are visual accents; the existing continuous damage model remains unchanged.


### Loadout pools and ability resources

Outside combat, open a companion's Loadout and select a pool. Left and right spell pools
accept multiple spells; selected spells remain available to Skyrim's combat AI, which
chooses among them for the situation. Teaching a player's spell still teaches it permanently.
A two-handed ritual is selected in both hand pools. Weapon pools list carried weapons (staves for casters, two-handed melee weapons for Bruisers);
selection limits the AI to those weapons, without removing anything from inventory.
Empty pools mean automatic choice. Use Done or Escape to close; selection stays open.
Existing single-spell choices migrate into one-item pools. Pools persist in the co-save.

Bruiser requires a carried greatsword, battleaxe, or warhammer to select the role, spend
points in its tree, or activate its abilities. While assigned Bruiser, only two-handed
melee weapons may be used. Losing the required equipment makes the path unavailable;
purchased ranks remain saved. Other weapon pools never override this restriction.

Active abilities spend 10% / 15% / 20% of maximum resource at ranks I / II / III.
Healers and Mages spend magicka; other roles spend stamina. Insufficient resource blocks
both automatic and manual activation. Failed activations cost nothing; successful casts
pay once, with no extra cost for individual pulses or follow-up impacts. Normal equipped
spells keep their normal Skyrim spell costs. Passive traits do not pay activation costs.
The old flat Taunt stamina setting is retained for save/config compatibility but no longer
sets the cost. Without Paths, resource-cost tiers use follower levels 1 / 30 / 60.

Dragon's Breath's fire stream now has denser textured flame layers at every rank.

Ability level gates: existing purchases below level 35/50 are refunded on load, along with their dependent branches; eligible purchases remain. Skill point costs remain 1/2/3 for abilities and 1 per branch rank.


Azura's Moon replaces Healer Ward as the level-50 fourth ability. Renewal is second and Rejuvenate is the level-35 third ability. The blessing grants a 20/25/30% chance to negate hostile weapon damage or a newly applied harmful health spell, for 10/12/14 seconds, within 12/14/16 metres. Moon's Reach adds 2/4/6 metres; Lingering Grace adds 2/4/6 seconds. Cooldown: 90 seconds; normal ranked Magicka activation costs apply. Overlapping blessings retain the strongest chance and longest duration, without adding probabilities. Existing Ward purchases are refunded on load.

The Wispmother rises in an authored praying pose. Ranks Iâ€“II are blue beneath a glowing moon; rank III is purple beneath the stock Azura's Star artifact. The outward pulse grants the blessing as it reaches each ally. A following ethereal aura remains for the actual blessing duration and fades during its final second. Effects and sounds can be disabled without disabling protection. Damage is intercepted before application; environmental damage and healing are not changed. Physical/enchantment components arriving within 50 ms share one roll. Damage from unrelated mods that directly writes health outside Skyrim's weapon/active-effect paths is not intercepted. Logs report recipients and prevented hit counts for playthrough verification.

## Latest effects and split review (2026-10-24)

Disarm, Second Wind, Azura's Moon and Riposte have their custom effects. Volley
emits a brief wind burst from the bow on every shot during its active window.
Explosive Arrow replaces Deadeye while retaining the old save identifiers and ranks.
An enemy impact triggers Skyrim's native fireball explosion artwork, area magic damage
(24/36/50 before resistance and falloff, radius 3/3.5/4 m), and stagger. Its generated
rune spins behind the archer and flashes on impact: emerald at I-II, emerald/crimson at III.
Blast Radius adds 0.5/1/1.5 m; High Explosive adds 10/20/30% explosion damage.

All effect updates run in Paths. The core never ticks abilities. See the shared checkout's
`docs/RELEASE-NOTES.md` for release changes and installation guidance.

## Windstep (2026-10-24)

Windstep replaces Pinning Shot in the third Archer slot (level 35). Old purchases,
branches and equipped slots carry over without resetting any other tree. Its cooldown
is 25 seconds; it uses the normal rank-based stamina cost (10/15/20% maximum).

| Rank | Maximum retreat | Speed duration | Incoming damage reduction |
| --- | --- | --- | --- |
| I | 3 m | 2 s | None |
| II | 3.5 m | 3 s | 20% |
| III | 4 m | 4 s | 30% |

All ranks grant +30% movement speed and leave a fading spectral archer with a bow.
Tailwind extends the speed/protection by 1/2/3 seconds; Fleet Footed raises the speed
bonus to 40%. Rank III layers the echoes and sends a 3 m stagger gust from the departure
point after 6 seconds, as the image begins its final fade. This gust deals no damage and cannot hit the party.

Auto use requires nearby pressure on the archer (a close attacker, a crowd, or low
health); distant enemies and attacks on unrelated allies alone do not trigger it.
The retreat follows a clear, grounded corridor away from the nearest threat. Blocked
casts spend no stamina and start no cooldown; automatic path retries are throttled.
The engine's colliding movement is retained, with ground/wall/occupancy probes before
and during travel. Interruption stops the movement, while a surviving follower keeps
the timed defensive buff. The image holds for 4/5/6 seconds and fades for 0.9 seconds.
An invisible temporary target under it distracts enemies already attacking the archer
within 20 m and line of sight. The normal target selector keeps them on the echo
through its fade; a later tank taunt can take precedence. It cannot attack, be looted,
join the roster, or be picked as a party enemy. Expiry, caster/role loss, cell changes,
saving and reset remove the target and its holds. Debug casts remain visual-only.
The archer is not made invisible and damage/protection values are unchanged.

Effects 1.0.19 replaces the procedural figure with Skyrim hide-armour male/female
archers: cuirass/body, fitted gauntlets/hands, boots, helmet, head, and iron bow.
Stock skinned geometry is baked from frame 0 of Skyrim's `bow_idledrawn.hkx`,
including its separate bow animation, left-hand Shield attachment, and right-hand
Weapon attachment for the nocked iron arrow. No manual arm posing is used; only the transformed
spell geometry ships, with diffuse references resolving to Skyrim's own textures.
The runtime chooses the female version when the follower's NPC base is female,
otherwise male. This is a generic bandit echo, not the follower's face, race or gear.
Each variant has eight independent copies. The stock clothing UVs remain fixed;
only the wind textures scroll. All parts fade with the existing spectral animation.

Preview: `http://127.0.0.1:8471/windstep.html?rank=3`, with a male/female selector.
It uses the shipped geometry and animation; the solid stock archer shows retreat.
Native collision, movement, stagger, damage reduction and actual enemy acquisition still require an in-game playtest. Target samples are logged under `[Windstep]`; see `tools/windstep/README.md` for the validation checklist.

Theme changes made under **Path of the Companions > Settings > Panel > Theme** also update Fight With Me when both are installed. Without Fight With Me, the theme is saved in `FightWithMePaths_user.ini`.

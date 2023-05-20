# [L4D2] Improved Survivor Bots
This is a SourceMod Extension that fixes lots of bugs and improves the existing behaviors of the survivor bots. This project was made around April 2020 and had been in development for about a year but has been **abandoned** meanwhile since I don't have the time to maintain it. You might have to modify the source code in order to compile it.

It was originally made for the Linux version of the game but later received Windows support as well; however, there's an issue with custom behaviors on Windows which causes a server crash. 

I suggest not using this extension on your server but instead, creating a fork and editing it or using the code and creating your own version with it. You can also use [this plugin](https://forums.alliedmods.net/showthread.php?p=2771953) as an alternative if you want to modify behaviors using plugins.

If you decide to use it anyway, make sure not to use my other [survivor bot fixes](https://github.com/justin-chellah?tab=repositories) since these fixes are already included in this extension and might cause issues.

# Fixed Issues
```
- Bots go after teammates even though they're very far away from the rest of the team
- Bots prioritize teammates in a strict order instead of rescuing the ones that are the closest to them
- Bots retreat from a Witch even when she's busy attacking another survivor
- Bots retreat from a Charger even when it's busy with another survivor
- Bots abandon their human lead player to pick up nearby items when regrouping with them
- Bots wouldn't avoid nav areas even though they're marked with the "AVOID" attribute
- Bots switch between secondary and primary weapons too frequently whenever they're aiming at infected on different ranges
- Bots wouldn't stop moving when nav areas are marked with the STOP attribute
- Bots walk into acid spit or fire instead of waiting it out before continuing to move
- Bots wouldn't jump when nav areas are marked with the JUMP attribute
- Bots jump despite nav areas being marked with the NO_JUMP attribute
- Bots stand still while consuming pain pills/adrenaline which made them easy targets
- Bots give up battle stations to cover another survivor in combat
- Bots give up battle stations to regroup with their human leader regardless of the give-up range cvar
- Bots repeatedly move from left to right when they're unable to find a path to the player they're trying to heal or rescue
- Bots chase after another survivor with a first aid kit despite their range limit being exceeded
- Bots continue healing themselves or another player despite a Special Infected being nearby
- Bots continue healing themselves or another player despite another survivor being incapacitated or dominated by a Special Infected
- Bots continue using pain pills or adrenaline despite being healed by another player
- Bots continue using pain pills or adrenaline despite a Special Infected being nearby
- Bots continue using pain pills or adrenaline despite another survivor being incapacitated or dominated by a Special Infected
- Bots chase after another survivor with pain pills or adrenaline despite their range limit being exceeded
- Bots continue trying to give pain pills or adrenaline to another player despite the infected being nearby
- Bots continue trying to give pain pills or adrenaline despite another survivor being incapacitated or dominated by a Special Infected
- Bots continue the Action to rescue teammates despite that teammate no longer being in trouble
- Bots tap-fire chargers instead of spraying them when using assault rifles
- Bots continue reviving someone even when the game told them to stop (Revive progress never ended)
- Bots never switch back to a weapon after an unsuccessful healing attempt of themselves or another player
- Bots give up too easily reviving another player when taking damage from infected
- Bots give up too easily healing themselves or another player when taking damage from infected
- Bots stand still for one second whenever they try to kill or shove a Jockey
- Bots have trouble freeing survivors that are hanging from the tongue or are still being dragged
- Bots continue the Action to revive teammates despite that teammate no longer being in trouble
- Bots never execute the Action to rescue teammates from jockeys, hunters, and smokers
- Bots only check the ammo count of the weapon that's currently being held when trying to collect ammo
- Bots try to ambush dead boomers
- Bots stop specific Actions when they or the player they're interacting with got vomited on by a Boomer
- Bots often not jump on certain ledges and props
- Bots walk when nav areas are marked with the WALK attribute
```

# Requirements
- [SourceMod 1.11+](https://www.sourcemod.net/downloads.php)
- [Matchmaking Extension Interface](https://github.com/shqke/imatchext)

# Docs
- [NextBot](https://developer.valvesoftware.com/wiki/NextBot)
- [NextBot SDK](https://github.com/SourceSDK2013Ports/NextBot)
- [The AI Systems of Left 4 Dead](https://steamcdn-a.akamaihd.net/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf)

# Supported Platforms
- Windows
- Linux

# Supported Games
- Left 4 Dead 2

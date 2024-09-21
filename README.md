# Intro
I used to get pretty hyped when respawn had announced their decision of enabling proton support. My intrinsic motivation has caused me to spend an excessive amount of time working on this project. I am not just publishing this because i feel burnt out. But also because it certainly may be of use for someone out there.
I personally haven't seen much public work in this field, especially not anything that goes too much beyond surface level. Writing cheats for anticheat protected games requires you to think creatively. If you proceed at this important step by doing what everyone else does, you are toast without even having entered the battle yet.

# Status
My intent is not to influence the cheating situation of a game in a negative way. But because of my experience in this field, i would argue that i can pretty accurately classify various types of information. 
My project is not making use of any ground breaking technologies, and will not work on latest game build. If a cheater is skilled enough to update this project, then he will be able to make quick work of proton EAC, even without my help.

# Components
In terms of complexity, DrauseHook is laid out in a rather simplistic manner. Following components are included:
- **loader:** it will act as a bootstrapper and initialize the cheat payload.
- **payload:** will be loaded into the game's context, contains actual cheat functionality.
- **backend:** will not be shipped in public release, but basically acts as the counterpart that is (normally) required for successful injection.

You do not have to get in panic because of backend not being released, offline mode was a feature already!

# Configuration
It is possible to customize injection to some extent. Currently, there are following options available:
- **Payload Debugging** (DEPRECATED) | --debug-payload | reads from a designated memory region within the active payload and outputs diagnostic data for cheat payload in console.
- **Debug Signatures** | --debug-sigs |
- **Dump Game Image**  | --dump-game-image  | creates an in-memory dump of the game, in this case apex legends, and saves it onto your disk.
- **Analysis From Dump**  |  --analysis-from-dump  | does analysis for offsets finding on a dump file, instead of pulling it directly from mem.
- **Custom Payload Path**  |  --custom-payload-path  |  allows one to specify a custom cheat payload path.
- **Use Unsafe Memory** (UNSAFE) |  --use-unsafe-memory  |  will allocate new executable memory and map cheat payload into that.

# Intro
I used to get pretty hyped when respawn had announced their decision of enabling proton support. My intrinsic motivation has caused me to spend an excessive amount of time working on this project. I am not just publishing this because i feel burnt out. But also because it certainly may be of use for someone out there.
I personally haven't seen much public work in this field, especially not anything that goes too much beyond surface level. Writing cheats for anticheat protected games requires you to think creatively. If you proceed at this important step by doing what everyone else does, you are toast without even having entered the battle yet.

# Media
- https://www.youtube.com/watch?v=uA9x9bFkLGk
- https://www.youtube.com/watch?v=rIjyrj1-jKA
- https://www.youtube.com/watch?v=rJFViLL85Qk
- https://www.youtube.com/watch?v=hdSVMwF9Njs

# Status
My intent is not to influence the cheating situation of a game in a negative way. But because of my experience in this field, i would argue that i can pretty accurately classify various types of information. 
My project is not making use of any ground breaking technologies, and will not work on latest game build. If a cheater is skilled enough to update this project, then he will be able to make quick work of proton EAC, even without my help.

That being said, it does have a great amount of detection vectors covered already. Here is an overview:
1. **Stealth VMT-Hooking:** By placing our reallocated table within the bounds of R5Apex.exe, we are evading their bounds check. However, they could start restricting it to .rdata instead of whole module. Then you'd be fucked.
2. **Stealth Control Flow:** By tampering with their stack-walking algorithm in a stealth manner, we are able to completely disable it. Additionally, i have chosen to rely on hand written ASM trampolines for my hooks. This allows us to avoid having to invoke the original with a `CALL` instruction, which would make us visible to any external stackwalks, for example within original. Or something that it calls.

Obviously, there are still issues you'll need to take care of. <br>
A good start would be finding a better place to map than inside the `.data` region of `Compstui.dll`

# Features
- **Aim enhancement:** Allows for advanced enhancement of aim, either through A.) an unorthodox method of manipulating aim with mouse as input device, and B.) a similar functionality for controller, but using the game's own aim assist.
- **Visual enhancement:** Comes with a simple arrow key menu, ESP for players only (iirc), and some other minor stuff.
- **Movement helpers:** Offers various movement assistance in form of auto-superglide, auto-bunnyhop (buggy), and maybe some other shit which i've forgot.

You can also take a look at the source code if you want a more complete list of available features.

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

You can pass any of these as command line parameters when launching the loader. Example: `./loader --debug-sigs`

# Build Instructions
WIP

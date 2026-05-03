# An Education Barony DLC Unlocker
*I am not responsilbe for whatever happens when you use this, use at your own risk.*
## Information
This DLL is a simple DLC unlocker **you** attach to Barony that will init a console. *(As of now there are debug outputs in the game)*

This DLL file also frees certain restrictions and allows you to use **any** DLC character at **any time.**

**I will not provide a compiled DLL.**
## Changes
- Added pattern (AOB) scanning
- Changed method for unlocking DLC
- Added welcome message call utilizing the in-game "mono-prompt"
## Known Issues
Custom mono-prompt crash:
```
I'm not 100% sure why (this is a slop project), but calling the prompt provokes a crash upon cleanup.
I believe the offset is outdated, or the return type is incorrect.
Other than that, it's likely something to do with the way the updated engine handles prompts in specific menus.
```
> [!CAUTION]
> I am not responsible for what you do with this or the actions that follow!

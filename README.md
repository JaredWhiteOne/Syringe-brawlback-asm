# Syriinge Brawlback ASM
Experimental refactor of Brawlback ASM into a REL module file.

# Requirements
Note that this only works on Windows at the moment with Project+, but simply run make in the root directory after putting the Project+ files into a folder SDCard in the root. 

You will need to place BrawlbackLoader.asm into the SDCard/Project+/Source folder, add the line `.include Source/BrawlbackLoader.asm` to RSBE01.txt, and rerun GCTRM against that file to produce a modified RSBE01.GCT. 

Configure and use MakeSD as desired via tools/MakeSD/Config.ini to build the SD card; note that this can be done in Dolphin as well, it's just faster (and only available on Windows).

Finally, you will need to expand MEM2 in Dolphin to 128MB under the Advanced tab on the Configure submenu.

Use the branch `unstable/syringe-refactor` for Dolphin; you must have 2 Dolphin directories and executables, as well as 2 SD cards to test the netcode.

To run MakeSD, run the command "./tools/MakeSD/main.bat" from PowerShell in the root.

# Acknowledgements
DukeItOut for GCTRM

[Sammi Huski](https://github.com/Sammi-Husky) for Syriinge and [fudgepop01](https://github.com/Fracture17/ProjectMCodes/tree/master/Codes/SuperTraining) for tutorials and great code examples

Everyone involved with BrawlHeaders, as well as open_rvl for their header implementations

The PMDT and P+DT as well as all those who have contributed code in the Project+ Source

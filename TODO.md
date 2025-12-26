DESIGN_PHILOSOPHY_1 (DP1)					: Within a Frame (example: per VBLANK), CPU, DISPLAY, AUDIO, TIMER, INTERRUPT, I/O are handled completely independently
DESIGN_PHILOSOPHY_2 (DP2_1)					: Within a Frame (example: per VBLANK), CPU is master the remaining peripherals i.e. DISPLAY, AUDIO, TIMER, INTERRUPT, I/O are handled within the CPU Ticks
											The DP2_1 design philosophy is as follows:
											1) CPU tick will be done one cycle at a time; to increment multiple times, call the inc func multiple times
											2) Inside this tick function, other ticks will be handled (ppu ticks, audio ticks, timer ticks, dma ticks etc)
											3) As cpu tick will be done one t cycles at a time, during processing of each opcode, we can do cpu ticks (and inturn ppu/apu/timer/dma ticks) in between memory fetchs and memory writes ensuring accurate memory timing
DESIGN_PHILOSOPHY_2 (DP2_2)					: Break down the opcode to micro fsm i.e. opcode pipeline fsm and run them similar to the way we run the opcode fsm, but we would need sort of co-routine to run the sub fsm
											i.e. a switch case opcode code pipeline fsm inside the switch case opcode fsm -> hence a co-routine
											Refer to https://www.reddit.com/r/EmuDev/comments/a7kr9h/comment/ec3wkfo/?utm_source=share&utm_medium=web2x&context=3	
											Refer to https://www.geeksforgeeks.org/coroutines-in-c-cpp/
											Refer to https://stackoverflow.com/questions/652815/has-anyone-ever-had-a-use-for-the-counter-pre-processor-macro
											Most common way to do this is probably via MACROs and GOTO functions with __COUNTER__ for unique label generation
DESIGN_PHILOSOPHY_2 (DP2_3)					: Break down the opcode to micro fsm i.e. opcode pipeline fsm and run them similar to the way we run the opcode fsm
											Two fms will run on its own thread and sync will be via semaphore or mutex
DESIGN_PHILOSOPHY_3 (DP3)					: Implement a scheduler based on cpu or ppu or apu tick (whichever has relatively lower periodicity) and this scheduler triggers event when the tick reaches target										
				
PROTOTYPE-0001 								: Design philosophy used is DP1 and implementation uses OLCPixelGameEngine and uses
PROTOTYPE-0002 								: Design philosophy used is DP1 and implementation uses SDL2
PROTOTYPE-0003 								: Design philosophy used is DP1 and implementation uses ImGUI + SDL3
PROTOTYPE-0010 								: Design philosophy used is DP2 and implementation uses OLCPixelGameEngine and the OLC thread for emulation frame
PROTOTYPE-0022 								: Design philosophy used is DP2 and implementation uses OLCPixelGameEngine but instead of OLC thread, uses audio thread for emulation frame
PROTOTYPE-0052 								: Design philosophy used is DP2 and implementation uses ImGUI + SDL3 and uses audio thread for emulation frame
PROTOTYPE-0110 								: Design philosophy used is DP3 and implementation uses OLCPixelGameEngine and the OLC thread for emulation frame
PROTOTYPE-0122 								: Design philosophy used is DP3 and implementation uses OLCPixelGameEngine but instead of OLC thread, uses audio thread for emulation frame
PROTOTYPE-0152 								: Design philosophy used is DP3 and implementation uses ImGUI + SDL3 and uses audio thread for emulation frame

#CURRENT*******************************************************************************************************************************************************************************************************************
***PAGE3*******************************************************************************************************************************************************************************************************************
***NOT-COMPLETED***********************************************************************************************************************************************************************************************************

**INFORMATION***********************************************************************************************************************************************

* Going forward, all bug fixes or feature addition to the following platforms will be done ONLY in masquerade's P0052 or P0152 builds:
1) Game Of Life
2) Chip8
3) Space Invaders
4) PacMan
5) Ms PacMan
6) Nintendo Entertainment System
7) Gameboy
8) Gameboy Color
9) Gameboy Advance

**TODOs*****************************************************************************************************************************************************

* masquerade (P0152)						-> Implement
                                            -> Issues
                                            1) Current implementation of cycle accurate is not correct and mid instruction timing is not handled
                                            2) Currently making ppu/apu/sio/timer cycle accurate makes it very slow
                                            
                                            With scheduler, if we just have mode events for PPU, then we would loose mid scanline updates, how to handle this? check discord and NBA
                                               

* masquerade (P0152)/gameboy advance		-> Integrate ARM7TDMI SST (https://github.com/SingleStepTests/ARM7TDMI)
* masquerade (P0152)/gameboy advance		-> Pokemon Emerald ingame save fails (passes few times at the start, then it fails continously)
* masquerade (P0152)/masquerade-qa			-> Create automated gba tests

* masquerade (P0152)						-> Android Support

* masquerade (P0152)/COSMAC VIP				-> Implement

* masquerade (P0152)/Atari 2600				-> Implement

* masquerade (P0152)/ZX Spectrum 			-> Implement
* masquerade (P0152)/Commodore PET 			-> Implement
* masquerade (P0152)/TRS-80 				-> Implement
* masquerade (P0152)/Apple II 				-> Implement
* masquerade (P0152)/Atari 400/800			-> Implement
* masquerade (P0152)/IBM PC 				-> Implement
* masquerade (P0152)/Commodore 64 			-> Implement

* masquerade (P0152)/gameboy advance		-> Dragon Ball GT video rom (and many more) is not playable in GBA (Check if video DMA is being used)
*											-> Dragon Ball GT is an exception to this -> No$GBA seems to be having same issue and its also logging some errors, use this as clue!
*											-> Dillonb seems to be going little further with Dragon Ball GT, so maybe need to compare the logs
* masquerade (P0152)/gameboy advance		-> Total GBS is not running in GBA
* masquerade (P0152)/gameboy advance		-> Implement MOSAIC mode and FORCE BLANK modes
* masquerade (P0152)/gameboy advance		-> Fix the Mode 3 top left corner glitch
* masquerade (P0152)/gameboy advance		-> Pass AGB/AGS timer prescalar 0 test

* masquerade (P0152)						-> Support Rewind

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

* masquerade (P0152)/GB/GBC					-> Serial implmentation in Prototype-52 branch for GB/GBC
* 											-> Serial Clock is also based on DIV similar to Audio as per AntonioND's doc. Implement this!
*											-> Serial Link, looks like is showing latency issues with ASIO. Check how pyboy intends to implement this?

* masquerade (P0152)/GBC					-> MMIO_exec_1.gb should pass in CGB as well
*											-> Looks like this might be related to ppu timing diffs between CGB and DMG
*											-> We see in DMG, FF06 is INC C (0xC) but in CGB, its DEC C (0xD)
*											-> We also see more loops in cgb before FF06 check whereas these extra loops is after the FF06 check in DMG
*											-> Basically the loop in question is checking for LY = 144, so some LY timing...

* masquerade (P0152)/gameboy advance		-> Idle loop detection

* masquerade (P0152)/Game Of Life			-> Scalable Game Of Life with Pan/Zoom (similar to OLC https://www.youtube.com/watch?v=OqfHIujOvnE&ab_channel=javidx9)
* masquerade (P0152)/Game Of Life			-> Accept commands from CLI to set pattern, for example -> b1d(60, 49, L"##........#.....#...##...............");

* masquerade (Web-P0152)					-> Web version is not able to load roms with
*											-> Names with capital letters
*											-> Names with special characters

* masquerade (Web-P0152)					-> GBA needs to work post enabling -sSAFE_HEAP=1 in emscripten
*											-> There is no issue in desktop version even when enabling CRT and sanitization checks
*											-> Most likely memory alignment checks are failing which is by default enabled with -sSAFE_HEAP=1
*											-> Just need to confirm this for now!

**DEFERRED**************************************************************************************************************************************************

* masquerade (P0152) 						-> screenshots and recording audio + video to file (using python scripts inside exe)

* masquerade (P0152)/Super Gameboy 			-> Implementation

* masquerade (P0152)						-> Add android support

#HISTORY*******************************************************************************************************************************************************************************************************************
***PAGE2*******************************************************************************************************************************************************************************************************************
***COMPLETED***********************************************************************************************************************************************************************************************************

**DEFERRED**************************************************************************************************************************************************

* audio 									-> Support "Sync to Audio Playback Rate"

* common									-> Need to add option for FPS Boost. 
*											-> Note that audio config and static buffers are setup assuming 60FPS.
*											-> We need a mechanism to handle this FPS change dynamically if we need FPS boost

* I8080/Z80/SM83/6502/ARM7TDMI 				-> Implement pending instructions and undocumented behaviour

* gameboy advance							-> GBA Audio
* 											-> Undesired artifacts should be removed; this is either because we dont resample or because of FPS bottleneck
* 											-> Most probably, we have hit the FPS limit which is becoming a bottleneck; need to move to P0152 to further enhancements to GBA Audio

* gameboy advance							-> Implement ROM prefetch, we may need to handle sub-opcode memory read/write timing, else we need to implement a scheduler
* gameboy advance							-> Pass all jsmola/alysoha-tas tests, we may need to handle sub-opcode memory read/write timing, else we need to implement a scheduler
* gameboy advance							-> Pass all AGB/AGS tests, we may need to handle sub-opcode memory read/write timing, else we need to implement a scheduler

* gameboy/gameboy color 					-> Pass all the scribbltests
* gameboy/gameboy color						-> Pass all moon-eye tests		
* gameboy/gameboy color						-> Pass all wilbertpol tests			
* gameboy/gameboy color 					-> Pass all Age tests
* gameboy/gameboy color 					-> Pass blargg's oam_bug test

* nintendo entertainment system				-> Implement all other mappers using rom database	
* nintendo entertainment system 			-> Pass mmc3_test_2's 4-scanline_timing.nes test
* nintendo entertainment system 			-> Pass all sprdma_and_dmc_dma test which requires properly emulating cycle accurate DMA i.e. DMA PUT/GET cycles w.r.t CPU READ/WRITE cycles

* Assembler									-> Assemble code in emulator and run in emulator!

* debugger (gameboy advance)				-> Implement Debugger
* debugger (gameboy/gameboy color)			-> Implement Debugger
* debugger (nintendo entertainment system)	-> Implement Debugger
		
* gameOfLife 								-> Optimization; refer to algorithm section in https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life

* masquerade 								-> Mods & Settings for games running in the emulator using Lua (refer javidx9 videos)
**INFORMATION**************************************************************************************************************************************************
-> Automated Tests: It's best if your emulator can automatically run a suite of tests at the press of a button. This allows you to re-run them every time you make a change, without any effort. Automation can be difficult, because the emulator must be able to determine success/failure without your help.
The first part of automated testing is support for a "movie" or "demo", or a list of what buttons were pressed when. An emulator makes a movie by recording presses while the user is playing, and then it plays the movie by feeding the recorded presses back through the input system. This not only helps automated testing but also makes your emulator attractive to speedrunners.
To create a test case, record a movie of the player activating all tests in a ROM, take a screenshot of each result screen, and log the time and a hash of each screenshot. The simplest test ROMs won't require any button presses. ROMs that test more than one thing are more likely to require them, and an actual game will require a playthrough. Then to run a test case, play the movie in fast-forward (no delay between frames) and take screenshots at the same times. If a screenshot's hash differs from that of the corresponding screenshot from when the test case was created, make a note of this difference in the log. Then you can compare the emulator's output frame-by-frame to that of the previous release of your emulator running the same test case. 
In spaceInvaders/pacman, see if we can separate out processDisplay and displayCompleteScreen for space invaders and pacman, similar to gb

-> One thing to investigate here is that processDisplay function for these 2 emulators take vram address as input 
so when during opcode processing, can we call the processDisplay for that particular vram address if it was modified by the current opcode and update the gfx buffer, 
and, displayCompleteScreen just displays this buffer

Provide emscripten support with CMAKE
-> conditional compilation to remove OLC Soundwaveengine and boost when __EMSCRIPTEN__ is defined
While removing boost, alternative is to define the default values for items present in CONFIG.ini file in defaults.h
How to change roms without rebuilding application again and again. At the moment, we need to put the rom in asset folder, and hardcode it in the code
and for a new ROM, new build needs to be done

-> Provide clang/LLVM support with CMAKE
Deferred for now as there is not much use/interest to this.

#HISTORY*******************************************************************************************************************************************************************************************************************
***PAGE1*******************************************************************************************************************************************************************************************************************
***COMPLETED***************************************************************************************************************************************************************************************************************

**INFORMATION***********************************************************************************************************************************************

* I/O processing:
-> To fix the Chip8 test issues (issues w.r.t to I/O), the opcode processing is done at max speed now and is at same rate as I/O processing. 
This was needed because if any opcode direclty supports I/O, then I/O processing is asynchronous and is dependent on opcode processing, i.e. instead of processing at fixed rate, it is done whenever the PC points to an opcode which processes the I/O, hence asynchronous.
The issue was very pronounced in chip8 and hence needed this fix where the opcode processing was brought down to match I/O processing speed. The reason I/O processing was not bumped up is because I/O processing is capped by OLC Frame rate
This issue can happen in other chips where asynchronous I/O processing which is dependant on opcode processsing is supported, but we have not had any observable issue so far.
The only fix possible for this is to have an instantaneous way of reading keys (or even outputing GUI) along with supporting updates at every frame; the later mentioned method is the only method OLC supports...

* Emulator BIT:
-> Any Game or Emulator as part of POST should exercise all the opcodes one after the other as soon as power on with some dummy data.
For example, if 256 opcodes, first 256 cycles should be dedicated for this exercise.
For emulation developement, it will help as using just 256 cycles, we can basically test most of the CPUs functionality

**ENHANCEMENT***********************************************************************************************************************************************

* More than one GUI window support in OLC ?... will we have have more than one "OnUserUpdate" methods ?

**DEFERRED**************************************************************************************************************************************************
The test rom outputs on the GUI rather than console? 
-> This is easily possible BUT slows down the test DRASTICALLY as we cannot remaing in "while loop" during test run because "OnUserUodate" needs to be called continously for GUI display

Create an external library for absolute save and absolute load states. Via runtime library, the support should be addable for any emulator
-> Created a separate project for this and the DLL was created; basics were tested but not integrated to emulator as it lead to some memory violation issue. Hence deferred for now.

#HISTORY*******************************************************************************************************************************************************************************************************************
***PAGE0*******************************************************************************************************************************************************************************************************************
***COMPLETED***************************************************************************************************************************************************************************************************************

**INFORMATION***********************************************************************************************************************************************

Build Environment:                                                                                                                                       
It is recomended to move to CMAKE.                                                                                                                       
For g++ compiler, MSYS2 based g++ is preferd when compared to CYGWIN based g++... from command line.                                                     
To select the MYSYS2 based compiler, the enviromental variable for MYSYS2 should be at higher position than CYGWIN                                       
 
**High Frequency CPU testing for Windows********************************************************************************************************************

The speed issue is resolved... refer to ***debug.txt*** for spaceInvaders

-> Let space invaders be slow.... but make it work for now. While debugging, no need to wait from start everytime, take advantage of save state and hence save just
   before the point which we need to debug. Do this first on Space Invader specific emulator instead of multi console

-> Start integrating one by one, very small functional parts of olc NES emulator in masquerade multi-emulator and see when the speed goes down when compared to the pure olc NES

Points (0) and (1) were tested out completely and (2), (3) and (4) were tested partially... different graphics libraries are producing same results, so  
the final conclusion is either 
(A) CPU code itself is not that efficient... 
(B) Its a PC specific issue (which is highly doubtfull as of now)
(*)One thing is download code for chip8 and spaceinvaders which uses SDL and has similar code to mine w.r.t to CPU and see how it runs in this PC

Point (0) seems to help resolve the SDL's "cursor howering over the x button" crash
0) http://www.sdltutorials.com/huh-my-pong-clone-uses-100-percent-cpu-and-is-still-slowhttp://www.sdltutorials.com/huh-my-pong-clone-uses-100-percent-cpu-and-is-still-slow
1) Try porting the code from olcPixelGameEngine to SDL and see if there is any difference (definetly easy to verifys since most online resources uses SDL)
	Start with Chip8 and move to SpaceInvader, if both indicate speeds are good, then move to Masquerade
2) Check whether we can use a different PROCESS for cpu rather than a thread (This is definetly needed for future emulators)
3) Investigate running the check for frequency of olcPixelGameEngine in other Systems. Slow rate might be an issue specific to My PC
4) Start the CPU thread before calling the Start() of olcPixelGameEngine (investigate where to call join())

* High Frequency CPU rate testing is being done under D:\WorkSpace\Emulators\Tests\features-testing\executionRate\ClockTesting
* Next option to try out is running CPU execution in WHILE(1) and rest others from different threads
* Since OLCPixelGameEngine has its own game thread, maybe create another thread with WHILE(1)

**TIMING SUPPORT IN C++ for Windows*************************************************************************************************************************
https://stackoverflow.com/questions/10905892/equivalent-of-gettimeday-for-windows
https://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter

**OPCODE PERFORMANCE****************************************************************************************************************************************
* Display is not causing the slow down as proved by below measurement. Other than opcode processing, nothing else is run at every frame...
* So, try measuring the frequence of every frame WITH and WITHOUT processOpcode
* Desired Frame frequency should be 2 MHz
* With Opcode Processing Enabled
AVG time: > 400 us (not even close to 2 MHz)
* With Opcode Processing Disabled
AVG time: > TBD

To test the perf measuring software, we will implement a simple while loop with sleep(x)

* Testing with while loop indicates that this method is not reliable and also timing of 2MHz is probably not achived

**DISPLAY PERFORMANCE***************************************************************************************************************************************
* Create separate individal emulators so that polymorphism will not come into picture and then test the speed of emulation
* Once this is done, measure time taken to blacken the screen with and without calling the display function. 
* For reference, maybe use cyclecount as threshold for stopping the time measurement
* Below mentioned values are the threshold
host cpu cycle:       37444
emulated cpu cycle:   275809
*** TRY 1:
*** With Display Enabled
Time: 72354.4
*** With Display Disabled
Time: 78120.5
*** TRY 2:
*** With Display Enabled
Time: 77534.7
*** With Display Disabled
Time: 94966.3
*** TRY 3:
*** With Display Enabled
Time: 87209.5
*** With Display Disabled
Time: 91659.2
**ISSUEs****************************************************************************************************************************************************
* maybe see how olcNES is able to maintain high CPU rate (resulting high gfx rate) by disabling/enabling parts of the olcNES code
* OLC NES used DrawSprite instead of Draw. Will this give the speed boost that is seen in olcNES ??? (internally Drawsprite uses Draw though...)
* Is some exterior program capping the speed ??? (similar to NVIDIA VSYNC)
* Create a separate thread for CPU emulation (completely independent of Graphics) so that..
* is there is any frame cap, that will not happen here, and
* we can see if we achieve 2 MHz CPU speed
* BUT for these, since CPU runs faster that graphics "BYTE PER FRAME" is not possible
* Will have to run "Buffer Per Frame"
* It looks like its SW issue or maybe frame cap since olcNES worked perfectly fine...
* For debugging (of opcodes), after openning emulator, load the save file which will jump to the end of screen initialization (black screen)

* SOLUTION 1 for speed issue: FPS is actually good, so display is fast, but its wasted as not enough info is available per single frame of masquerade
* So, CPU speed becomes bottleneck here
* Therefore, CPU has to run at much faster rate and then we can use the existing display methods in "Buffer Per Frame" mode
* One of the things to do is maybe create a standalone spaceinvader in c (or c++ with oops) and see if issue is there
* Other solution for this can be the "separate thread methods for the CPU mentioned above"
* OR
* run the masquerade class in a thread and CPU in while (true) loop
* If this doesn't work, one of the reasons could be that the CPU code itself is too slow...
Name:		Amar Gupta
Student No: 	500987921

I have completed this project individually.

I compiled and ran the program with Microsoft Visual Studio on Windows.

The main program is the game.cpp program, additional files required for the program to run are 
- vertexShader.vert (vertex shader), 
- fragmentShader.frag (fragment shader),
- cannonMesh.txt (output mesh form A2 for cannon),
- tiles.bmp (texture),
- clover.bmp (texture), and
- stb_image.h (for texture loading).

All of the above files MUST be in the same directory for the program to compile and run properly.

Notes:
- Defensive cannon is mouse controlled (note: the cursor must be over the game window for mouse control to work, if the cursor goes outside, it must be brought back inside the window).
- Defensive cannon is a custom mesh generated via A2.
- "SPACEBAR" key used to shoot from cannon.
- Collision detection is working for robots and defensive cannon.
- 3 Robots walk forward, and shoot randomly (accuracy should be about 10%).
- Defensive cannon has 3 HP, each hit will reduce it by 1 (displayed in terminal).
- When cannon HP is 0 or robots get past it, it is a loss and the cannon will disable and animate downwards.
- 'r' and 'R' restart the game.
- Vertex Shader and Fragment Shader are used (vertexShader.vert and fragmentShader.frag).
- Everything is texture mapped (defensive cannon, all parts of robot, projectiles, laser, floor, stars in background, etc.).
-'q', 'Q', and 'Esc' exit the program.

I have implemented all the asked functionality, as well as bonus (see below).

Bonus:
 - Cannon is a laser instead of a projectile
 - The robots will shrink and disappear upon being defeated.

The entire program should work.

If there are any problems at all feel free to email me at amar1.gupta@ryerson.ca.
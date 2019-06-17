# moe-engine

a 'no buzzwords' game-engine for 3D games written in C.

features: 
- 3D graphics and animation (including skeletal animation for models)
- hot code reloading of c-code
- basic collision detection and collision resolution (game-physics)
- in-game level-editor
- convenient performance timing
- debug renderer
- and more stuff

Basically this engine gives one the tools for building a game in a convenient fashion, while still giving the user full control over the exact architecture of every frame.

The hot code reloading allows for great code iteration of the gameplay code, while still giving us the performance, power, and familiarity of C.

# basic use:

Currently only works on Linux.
Compile by running build.sh in the testbed directory.
This should probably be enough.

To create a new 'level', create a folder in levels and insert a file 'buildlib.sh', which should build a shared object called 'shared.so'.
'buildlib.sh' will be called by the engine to build the 'shared.so' when reloading the code.
This shared object should include at least the functions:
void init_step(Game_Struct * game);
void gamestep(Game_Struct * game);

'init_step gets called after loading and reloading the code, and 'gamestep' gets called every frame after the last image has been rendered, and new input was polled.

Inside these functions you basically have all the freedom you want.
Take a look inside levels/testlevel1/gamestep.c for an example. 

Models you wish to render  after the gamestep just have to be pushed onto the models_to_render stack (their respective scaling should be pushed onto the models_to_render_scales stack).

To make this process more streamlined there is a datatype called 'Entity_Collection', which allows for easy instantiation of entities by their '.entity' filename, updating of entities by their id, collision against the set of entities and other often needed things.
This entity collection can be combined with other data to give it more potential behaviour.

# kinds of files:

'.moe'-files are the model/mesh format of the moe engine (models directory)
'.entity'-files describe basic entities (entities directory)
'.level'-files describe the layout of the static objects of a level, and describe a set of named points that can be queried in the code (a usage example being, spawning the player at a point called 'player_spawn')
'.obj'-files that can be used to describe physical objects via a set of triangles (physic_meshes directory)

# functionality:

Most of the code lives in the libs directory.
The main.c lives in the testbed directory, and gets compiled with build.sh.
The whole project is structured around the idea of having a single compilation unit, which allows for speedy compilation times and less compilation related bugs.
As such, the code lives in several .h files, grouped under common functionality. The names are partly inside-jokes still.
Some important ones are:

'render_basic.h' - Most things necessary for rendering, such as models, and node-hierachy related things. Just OpenGL currently.
'baka.h' - some physics code, such as code for bounding volume hierachies, raycasting, collision detection etc.
'mushishi.h' - debug rendering
'hashtable.h' - super barebones hashtable/map (since all strings entering it are in from a controlled environment not much more is necessary for now)
'functions.h' - some ungrouped general purpose functionality and some useful math


# coming features:
- easy to use in-editor light-baking and dynamic shadows
- AI pathfinding
- proper textured terrain
- sound
- More and better documentation will follow when the APIs become more fleshed out.

# thanks!

thanks to the following projects for providing helpful libraries under nice licenses:
https://github.com/skaslev/gl3w
https://github.com/HandmadeMath/Handmade-Math
https://github.com/rxi/microui
https://github.com/nothings/stb
https://github.com/RandyGaul/cute_headers
https://www.glfw.org/

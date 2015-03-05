# EsetBotWarz
ESET BotWarz competition entry framework

This is a framework that allows one to write bots for the ESET BotWarz competition in Lua. The framework takes care of the networking, login, command timing etc. The Lua AI controller just needs to set the commands to send to individual bots, and the framework will send them when it's time.

# Getting the sources
Either clone the repository using git, or download the ZIP file from the GitHub repo page.

If you cloned using git, you will need to init and update submodules, so that the additional libraries are pulled in:
```
git clone https://github.com/madmaxoft/EsetBotWarz .
git submodules update --init
```

If you downloaded the ZIP file, you will need to download the additional libraries (jsoncpp and libevent) and extract them at the appropriate subfolders in the `lib` folder. You can get the ZIP files for the libraries at each library's GitHub repo page, referenced directly in the `lib` folder:
https://github.com/madmaxoft/EsetBotWarz/tree/master/lib

# Compiling
To compile, you need a C++11-capable compiler (MSVC 2013 Express and gcc 4.8 were successfully tested) and CMake (2.8.12 was used for development)
The sources do support both in-source and out-of-source builds. You can either do:
```
mkdir Debug
cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
```
to get a Debug out-of-source build (usual for Linux), or do:
```
cmake .
```
to get a (default) in-source build (usual for MSVC)

# Running
The program itself needs several preconditions before it could be run. First, you need to create a file, `login.txt`, that will contain your login information for the competition. First line should be the login token, second line should be the login nickname. The file needs to be in the current directory when the program is run; most notably in MSVC you will want to set the current folder for debugging (rclk project -> Properties -> Configuration properties -> Debugging -> Working directory - set to `../out` ).

Next, you need to provide the Lua AI controller file that the program will use, as a command-line parameter. There is an example controller file in the `out/Controllers/Debugger` folder.

There are additional command-line options that could be helpful:
  - `/logcomm` makes the program write all communication with the server to a file
  - `/showcomm` shows all the communication with the server on stdout
  - `/pauseonexit` makes the program wait for an Enter keypress before exitting
  - `/nooutbuf` turns off runtime library's stdout bufferring (useful when redirecting stdout to another process)
  - `/zbsdebug` injects a small piece of code to the Lua controller code so that it can be debugged with ZeroBrane Studio (http://studio.zerobrane.com/)

# Writing Lua AI controller
The Lua AI controller is a single file that is specified on the executable's commandline, that the program uses to control the bots. It should define the following global functions, that are called when the specific event is received:
  - `onGameStarted(game)` - called when a new game is started, `game` is the table representing the game board
  - `onGameUpdate(game)` - called when the server sends an update (`play` response). `game` is the table representing the game board.
  - `onGameFinished(game)` - called when the server sends the game results (`finish` response). `game` is the table representing the game board.
  - `onBotDied(game, botID)` - called before `onGameUpdate` to notify that a bot (enemy or player) has died. `game` is the table representing the game board, `botID` is the numeric ID of the bot that has died. The bot is still present in the `game` table, but will be removed right after the callback function returns.
  - `onCommandsSent(game)` - called after the program reads the current bot commands and sends them to the server. `game` is the table representing the game board. The commands are already cleared when this callback is called.

The controller's job is to set commands for the bots in the `game.botCommands` table. Each bot will have an entry in the table, each entry will be a table with a `cmd` member and possibly the `angle` member (same meaning as in the BotWarz protocol). The program spawns a background thread that checks this table periodically (when the server is guaranteed to accept new commands), takes the commands that are currently present in the table, sends them to the server and clears the table. This means that the AI is free to leave any command in the table at any time, and they will be sent only when the server is guaranteed to accept the commands. Note that this means that the AI can put many commands there that simply won't get sent because they are overwritten before they are sent; this is a design choice and not a bug.

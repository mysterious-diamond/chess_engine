# A Simple Chess Engine
I was compelled to make this chess engine after seeing how easily a "newbie" did it on YouTube.
I also conviniently had a half year period before I go back to school, so here it is.

## Features
  - Two modes, **NORMAL MODE** where you can play with your friends (or yourself if you dont have any), and **BOT MODE** where you play against the actual chess engine
  - The chess is complete, castling, promoting, and even En Passant.
  - This engine is made by cross linking cpp and rust
  - No external libraries needed, all is included inside the repo. 
  - Completely open source, feel free to tweak in any way you like.
  - Made using the **RAYLIB** library. Its very cool, props to raysan5, it's creator. Check it out on https://github.com/raysan5/raylib.
  - **WINDOWS ONLY!! THIS PROJECT USES NAMED PIPES WHICH IS EXCLUSIVELY A WINDOWS ONLY FEATURE**

## How it works
It searches 5 moves deep for the highest value in a custom made "Evaluation" function to determine the best course of action
This is a very simple implementation of a chess engine, however, it is surprisingly effective.

To run, you can just grab the 2 executables, one is the interface/build/chess_engine.exe, 
then just run them. (OTHER EXECUTABLE W.I.P.)


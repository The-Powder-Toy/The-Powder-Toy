The Powder Toy - February 2019
==========================

Get the latest version [from the Powder Toy website](https://powdertoy.co.uk/Download.html).

To use online features such as saving, you need to [register an account](https://powdertoy.co.uk/Register.html).
You can also visit [the official TPT forum](https://powdertoy.co.uk/Discussions/Categories/Index.html).

Have you ever wanted to blow something up? Or maybe you always dreamt of operating an atomic power plant? Do you have a will to develop your own CPU? The Powder Toy lets you to do all of these, and even more!

The Powder Toy is a free physics sandbox game, which simulates air pressure and velocity, heat, gravity and a countless number of interactions between different substances! The game provides you with various building materials, liquids, gases and electronic components which can be used to construct complex machines, guns, bombs, realistic terrains and almost anything else. You can then mine them and watch cool explosions, add intricate wirings, play with little stickmen or operate your machine. You can browse and play thousands of different saves made by the community or upload your own – we welcome your creations!

There is a Lua API – you can automate your work or even make plugins for the game. The Powder Toy is free and the source code is distributed under the GNU General Public License, so you can modify the game yourself or help with development. TPT is compiled using scons.

Build instructions
===========================================================================

    sudo apt-get install build-essential libsdl2-dev libbz2-dev zlib1g-dev liblua5.1.0-dev git scons libfftw3-dev libcurl4-openssl-dev
    scons

For a list of flags for `scons`, see [the Powder Toy wiki](https://powdertoy.co.uk/Wiki/W/Scons_command_line_flags.html).

Thanks
===========================================================================

* Stanislaw K Skowronek - Designed the original
* Simon Robertshaw
* Skresanov Savely
* cracker64
* Catelite
* Bryan Hoyle
* Nathan Cousins
* jacksonmj
* Felix Wallin
* Lieuwe Mosch
* Anthony Boot
* Matthew "me4502"
* MaksProg
* jacob1
* mniip


Instructions
===========================================================================

Click on the elements with the mouse and draw in the field, like in MS Paint. The rest of the game is learning what happens next.


Controls
===========================================================================

| Key                     | Action                                                          |
| ----------------------- | --------------------------------------------------------------- |
| TAB                     | Switch between circle/square/triangle brush                     |
| Space                   | Pause                                                           |
| Q / Esc                 | Quit                                                            |
| Z                       | Zoom                                                            |
| S                       | Save stamp (use with Ctrl when STK2 is out)                     |
| L                       | Load last saved stamp                                           |
| K                       | Stamp library                                                   |
| 0-9                     | Set view mode                                                   |
| P / F2                  | Save screenshot as .png                                         |
| E                       | Bring up element search                                         |
| F                       | Pause and step to next frame                                    |
| G                       | Increase grid size                                              |
| Shift + G               | Decrease grid size                                              |
| H                       | Show/Hide HUD                                                   |
| Ctrl + H / F1           | Show intro text                                                 |
| D / F3                  | Debug mode (use with Ctrl when STK2 is out)                     |
| I                       | Invert Pressure and Velocity map                                |
| W                       | Cycle gravity modes (use with Ctrl when STK2 is out)            |
| Y                       | Cycle air modes                                                 |
| B                       | Enter decoration editor menu                                    |
| Ctrl + B                | Toggle decorations on/off                                       |
| N                       | Toggle Newtonian Gravity on/off                                 |
| U                       | Toggle ambient heat on/off                                      |
| Ctrl + I                | Install powder toy, for loading saves/stamps by double clicking |
| Backtick                | Toggle console                                                  |
| =                       | Reset pressure and velocity map                                 |
| Ctrl + =                | Reset Electricity                                               |
| \[                      | Decrease brush size                                             |
| \]                      | Increase brush size                                             |
| Alt + \[                | Decrease brush size by 1                                        |
| Alt + \]                | Increase brush size by 1                                        |
| Ctrl + C/V/X            | Copy/Paste/Cut                                                  |
| Ctrl + Z                | Undo                                                            |
| Ctrl + Y                | Redo                                                            |
| Ctrl + Cursor drag      | Rectangle                                                       |
| Shift + Cursor drag     | Line                                                            |
| Middle click            | Sample element                                                  |
| Alt + Left click        | Sample element                                                  |
| Mouse scroll            | Change brush size                                               |
| Ctrl + Mouse scroll     | Change vertical brush size                                      |
| Shift + Mouse scroll    | Change horizontal brush size                                    |
| Shift + R               | Horizontal mirror for selected area when pasting stamps         |
| Ctrl + Shift + R        | Vertical mirror for selected area when pasting stamps           |
| R                       | Rotate selected area counterclockwise when pasting stamps       |



Command Line
---------------------------------------------------------------------------

| Command                  |                                                                |
| ------------------------ | -------------------------------------------------------------- |
| scale:<size>             | Change window scale                                            |
| kiosk                    | Fullscreen mode                                                |
| proxy\:server[:port]      | Proxy server to use [Example: proxy\:wwwcache.lancs.ac.uk:8080] |
| open <file>              | Opens the file as a stamp or game save                         |
| ddir <directory>         | Directory used for saving stamps and preferences               |
| ptsave:<save id>#[name]  | [Example: ptsave:2198#Destroyable_city_5_wth_metro~dima-gord]  |

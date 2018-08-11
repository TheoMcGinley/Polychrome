# Polychrome

Polychrome is an experimental floating window manager designed for larger displays with a focus on movement and placement efficiency, and ease of window selection. 

Windows are framed with one of a set of colours, and can be immediately selected by pressing the hotkey associated with the colour of the border, allowing for windows to be easily selected regardless of the position of the previously selected window.

Polychrome uses a sloppy, grid-based window placement system, logically dividing the screen into a grid. Users can (optionally) specify a size and orientation for a new window using hotkeys - for example, `mod+m` sets the new window to be of medium size, and `mod+p` sets the new window to be a portrait window (if these are not specified, the default requested size of the window is used). Polychrome automatically places the new window in a location that minimises overlap using the grid system, meaning that minimum time and energy is spent manually organising the windows to not overlap. Note that tiling window managers promise the same reward, but do so in a much more restrictive manner, and with limitations - the tiling paradigm falls apart on larger monitors.

## Features
* Sloppy grid-based window placement: The best of both the floating and tiling window manager worlds, without compromise
* Window focus on colour: An easier, more intuitive way to select windows
* Virtual desktops: users can swap between many virtual desktops, allowing for effective window management without the need for minimization (and subsequently a taskbar)
  * Stickiness: windows can be assigned as "sticky", persisting between workspaces
  * Bezelless mode: As an alternative to multiple monitors, a fixed portion of the screen can be assigned as sticky, effectively allowing that portion to act as another screen (windows within the allocated space persist regardless of what actions are happening on the rest of the screen)
* pseudo-fullscreen: allows for centering and enlarging a window without actually allowing the window to take up the entire screen (useful for larger monitors)
* anti image retention: an option to slightly move all windows on a regular basis, preventing image retention and burn-in problems that can often be found in larger monitors

## Drawbacks
* Requires a non-trivial amount of learning to become adept with the window manager
* Requires (expects) users to customise to get the most from the window manager
* Does not accommodate for multiple monitors

# Meet HayBCMD
HayBCMD is a interpreter **highly** inspired by the console interpreter in games like Half-Life 2, Team Fortress 2(which are both made in the Source Engine) and DDNet(or Teeworlds but I did not play that version)

# How aliases works
```cpp
alias !echo4EvrLmao "echo ez banana"
> !echo4EvrLmao
> ez banana
> ez banana
> ez banana
> ez banana
> ...
> !echo4EvrLmao
>
> // stopped spamming "ez banana"
```

## in the console, the toggle alias is useless
```cpp
> alias +a "echo on"; alias -a "echo off"
> +a
> on
>
> -a
> off
```

## but in-game it's very important
```cpp
> alias +forward "echo walking forward"; alias -forward "echo stopped walking forward"
> bind w +forward
> w pressed
> walking forward
>
> w released
> stopped walking forward
> // it works on events: onKeyPress and onKeyRelease
```

> NOTE: console might be better off in a separate thread of the game
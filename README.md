Milkwave is:
- **Milkwave Visualizer**:
    A feature-enhanced version of [MilkDrop2](https://www.geisswerks.com/milkdrop/) visualizer
- **Milkwave Remote**:
    A standalone window which allows you do do things like send customized messages (either directly by typing or from a script file), load and change presets, send common key combinations using buttons and more

![image](https://github.com/user-attachments/assets/aa9da133-0cd2-4fb1-8eb6-3cbc1cd3530d)

## Visualizer Features
 
* Display current track information and artwork from Spotify, YouTube or other media sources playing on your PC
* Change preset on track change
* Set window transparency, borderless, and clickthrough ("watermark mode")
* Customize font types and colors
* Over 5000 presets from skilled artists worldwide
* Improved window handling, input methods and stability
* Use independently or in combination with Milkwave Remote

## Remote Features

* Send custom message text, color, font size, position and other parameters
* Save parameter definitions as named styles for quick access
* Send messages, parameters, presets etc. automatically from a script file at configurable intervals
* Select preset files and send them to the Visualizer
* Change the used audio device on the fly
* Show the currently playing Visualizer preset in the Remote window
* Amplify the virtual audio signal to make the Visualizer more (or less) responsive
* Modify the currently playing preset's waveform type, color and alpha value
* Send common key combinations using buttons
* Customizable interface - show only what you need

There are many tooltips explaining all features when you hover over the form elements. If you have any questions, don't be afraid to [ask for support](#support)!

## New in Milkwave 2.0 (unreleased)

### Visualizer

- Use cursor keys for media playback control
- Set Min./Max. pixel shader version in settings.ini
  - Indicate upgraded version with Up Arrow before preset name (configurable)
- Extended help display (F1)
  - Now two pages with new and previously undocumented features

### Remote

- Preset tagging
  - Tag your presets with any number of words of your choice
  - Automatic buttons for your most used tags
  - Load preset lists based on tags
  - Saved in human-readable json file for easy sharing or backup
- Tabbed interface
  - Presets, Messaging, Wave manipulation
  - Use mouse or Ctrl-Tab to switch between tabs
- More wave manipulation options
  - Set Zoom, Warp, Rotation etc. in the running preset
  - Clear preset and start with a plain wave
- Improved accessibility when using large Windows fonts

## History

The original MilkDrop 2 WinAmp plugin created by Ryan Geiss was turned into a Windows standalone application by Maxim Volskiy as [BeatDrop](https://github.com/mvsoft74/BeatDrop) and been improved upon eg. in the [BeatDrop-Music-Visualizer](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer) and [MilkDrop3](https://github.com/milkdrop2077/MilkDrop3) projects.

## System Requirements

* Windows 10 or later
* DirectX 9 or higher - compatible GPU
* [Microsoft .NET Desktop Runtime 8](https://dotnet.microsoft.com/en-us/download/dotnet/8.0) (for Remote)

## Support

I may add new features or fix bugs if people are actually using this, so feel free to [open an issue](https://github.com/IkeC/Milkwave/issues) or join my [**Discord**](https://bit.ly/Ikes-Discord)!

This project incorporates the work of many different authors over the years, as listed below. Naturally, the entirety of this project is Open Source and there will never be a paid version of it.

However, if you're satisfied with the additions I made and you want to support my work, here's my [**PayPal account**](https://www.paypal.com/ncp/payment/5XMP3S69PJLCU). You'll be listed as a supporter within the next program release if you wish.

## Acknowledgements

Many thanks to:

* Ryan Geiss - [MilkDrop2](https://www.geisswerks.com/milkdrop/)
* Maxim Volskiy - [BeatDrop](https://github.com/mvsoft74/BeatDrop)
* oO-MrC-Oo - [XBMC plugin](https://github.com/oO-MrC-Oo/Milkdrop2-XBMC)
* Casey Langen - [milkdrop2-musikcube](https://github.com/clangen/milkdrop2-musikcube)
* Matthew van Eerde - [loopback-capture](https://github.com/mvaneerde/blog)
* Incubo_ - [BeatDrop-Music-Visualizer](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer)
* milkdrop2077 - [MilkDrop3](https://github.com/milkdrop2077/MilkDrop3)
* podenthusiast - [Milkwave Logo](https://www.freepik.com/author/podenthusiast/icons)
* and all the preset authors!

If you believe you should be mentioned here, please let me know.

## License

[license]: #license

Milkdrop Remote is licensed under the [Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/), meaning you may not use the material for commercial purposes. See LICENSE.txt for details.

Milkdrop Visualizer (as a BeatDrop fork) is licensed under the [3-Clause BSD License](https://opensource.org/licenses/BSD-3-Clause) with the exception of where otherwise noted. See LICENSE.txt for details.

Although the original Matthew van Eerde's [loopback-capture](https://github.com/mvaneerde/blog) project didn't explicitly state the license, the author has been kind enough to provide a license clarification [here](
https://blogs.msdn.microsoft.com/matthew_van_eerde/2014/11/05/draining-the-wasapi-capture-buffer-fully/). All changes in this repository to the original Matthew's code are published either under the terms of BSD license or the license provided by original author.

## Contributions

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above.

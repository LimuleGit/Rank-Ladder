# Rocket League Ladder Rank

A highly customizable BakkesMod plugin that displays your Rocket League rank, MMR (Matchmaking Rating), and rank progression directly on your screen during gameplay.

## Features

-  **Real-time MMR Display** - View your current MMR as it updates after matches
-  **Rank Progression** - See your current rank along with the next and previous ranks
-  **Multi-Playlist Support** - Works with all ranked playlists:
  - 1v1 (Duel)
  - 2v2 (Doubles)
  - 3v3 (Standard)
  - Hoops
  - Rumble
  - Dropshot
  - Snowday
  - Tournaments
- **Fully Customizable** - Adjust position, size, spacing, and opacity to your liking
- **Lightweight** - Minimal performance impact on your game
- **Auto-Update** - Automatically refreshes after each match

## Installation

### Prerequisites
- Rocket League (Steam or Epic Games)
- [BakkesMod](https://www.bakkesmod.com/) installed and working

### Steps

1. **Download the Plugin**
   - Go in the repository
   - Download the latest `LadderRank.dll`

2. **Install the Plugin**
   - Navigate to your BakkesMod plugins folder:
```
     %APPDATA%\bakkesmod\bakkesmod\plugins
```
   - Copy `LadderRank.dll` into this folder

3. **Install Required Assets**
   - Download the `LadderRank` folder from the repository
   - Copy it to your BakkesMod data folder (if you don't have data folder create one):
```
     %APPDATA%\bakkesmod\bakkesmod\data\LadderRank
```
   - The folder structure should look like:
```
     data/
     └── LadderRank/
         ├── RankIcons/
         │   ├── 0.png
         │   ├── 1.png
         │   └── ... (all rank icons)
         └── RankNumbers/
             ├── 10.json
             ├── 11.json
             └── ... (all playlist data)
```

4. Load the Plugin (if you don’t see the plugin)
  - Open the BakkesMod Manager
  - Go to Plugins
  - Open the Plugin Manager
  - Check LadderRank
  - It should then appear on the left, and you can customize the plugin settings
    
    If nothing shows up:
  - Launch Rocket League
  - Open the BakkesMod console (F6 by default)
  - Type:
  - plugin load LadderRank
  - Or add this line to your cfg/plugins.cfg:
```
     plugin load LadderRank
```

## Usage

### Basic Controls

- **Enable/Disable**: Use the BakkesMod plugins menu (F2) → RankViewer → Toggle "Enable Display"
- **Access Settings**: F2 → Plugins → RankViewerNVersion

### Settings Overview

#### Display Options
- **Enable Display** - Show/hide the rank viewer
- **Show Next Rank** - Display the rank above your current rank
- **Show Previous Rank** - Display the rank below your current rank
- **Show Current Rank (Right)** - Display current rank on the right side
- **Show Current Rank (Left)** - Display current rank on the left side

#### Playlist Selection
Choose which ranked playlist to display:
- 1v1, 2v2, 3v3
- Extra modes (Hoops, Rumble, Dropshot, Snowday)
- Tournament ranks

#### Position & Layout
- **Horizontal/Vertical Offset** - Move the entire display around your screen
- **Rectangle Width/Height** - Adjust the size of the background
- **Left Margin** - Space from the left edge
- **Icon Size** - Size of rank icons
- **Icon Offset** - Distance of icons from edges
- **Text Offset** - Distance of text from icons

#### Spacing
- **Top Rank Spacing** - Vertical position of next rank
- **Middle Rank Spacing** - Vertical position of current rank
- **Bottom Rank Spacing** - Vertical position of previous rank

#### Customization
- **Opacity** - Background transparency (0 = transparent, 255 = opaque)

### Default Hotkeys

The plugin uses BakkesMod's standard menu controls:
- `F2` - Open BakkesMod menu

## Configuration

All settings are saved automatically through BakkesMod's CVar system. You can also manually edit them in the console:
```
// Enable/disable plugin
LadderRank_enabled 1

// Select playlist (11 = 2v2)
LadderRank_playlist 11

// Position
LadderRank_offset_x 700
LadderRank_offset_y -400

// Size
LadderRank_rect_width 470
LadderRank_rect_height 250

// Appearance
LadderRank_opacity 255
```

### Reset to Defaults

Click the "Reset to Default" button in the settings panel to restore all settings to their original values.

## Credits

- **Developer**: LimuleGit (ME)
- **BakkesMod**: [BakkesMod Team](https://www.bakkesmod.com/)
- **The Data folder**: Come to the pluggin LadderRank !!! It's a very cool plugin You need to check it don't hesitate : https://github.com/BeardedOranges/LadderRank

## Disclaimer

This is a third-party plugin and is not affiliated with or endorsed by Psyonix or Epic Games. Use at your own risk. While BakkesMod is generally safe to use, always ensure you're following the game's terms of service.

---

**Star ⭐ this repository if you find it helpful!**

Made with ❤️ for the Rocket League community

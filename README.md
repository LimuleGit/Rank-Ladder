# Rocket League Rank Viewer

A highly customizable BakkesMod plugin that displays your Rocket League rank, MMR (Matchmaking Rating), and rank progression directly on your screen during gameplay.

![Plugin Preview](assets/preview.png) <!-- Add your screenshot -->

## Features

- 📊 **Real-time MMR Display** - View your current MMR as it updates after matches
- 🎯 **Rank Progression** - See your current rank along with the next and previous ranks
- 🎮 **Multi-Playlist Support** - Works with all ranked playlists:
  - 1v1 (Duel)
  - 2v2 (Doubles)
  - 3v3 (Standard)
  - Hoops
  - Rumble
  - Dropshot
  - Snowday
  - Tournaments
- 🎨 **Fully Customizable** - Adjust position, size, spacing, and opacity to your liking
- ⚡ **Lightweight** - Minimal performance impact on your game
- 🔄 **Auto-Update** - Automatically refreshes after each match

## Screenshots

<!-- Add multiple screenshots showing different configurations -->
| Default View | Custom Layout | Settings Panel |
|-------------|---------------|----------------|
| ![Default](assets/default.png) | ![Custom](assets/custom.png) | ![Settings](assets/settings.png) |

## Installation

### Prerequisites
- Rocket League (Steam or Epic Games)
- [BakkesMod](https://www.bakkesmod.com/) installed and working

### Steps

1. **Download the Plugin**
   - Go to the [Releases](https://github.com/YourUsername/RocketLeagueRankViewer/releases) page
   - Download the latest `RankViewerNVersion.dll`

2. **Install the Plugin**
   - Navigate to your BakkesMod plugins folder:
```
     %APPDATA%\bakkesmod\bakkesmod\plugins
```
   - Copy `RankViewerNVersion.dll` into this folder

3. **Install Required Assets**
   - Download the `RankViewer` folder from the repository
   - Copy it to your BakkesMod data folder:
```
     %APPDATA%\bakkesmod\bakkesmod\data\RankViewer
```
   - The folder structure should look like:
```
     data/
     └── RankViewer/
         ├── RankIcons/
         │   ├── 0.png
         │   ├── 1.png
         │   └── ... (all rank icons)
         └── RankNumbers/
             ├── 10.json
             ├── 11.json
             └── ... (all playlist data)
```

4. **Load the Plugin**
   - Open Rocket League
   - Open BakkesMod console (F6 by default)
   - Type: `plugin load rankviewernversion`
   - Or add to your `cfg/plugins.cfg`:
```
     plugin load rankviewernversion
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
- `F6` - Open BakkesMod console

## Configuration

All settings are saved automatically through BakkesMod's CVar system. You can also manually edit them in the console:
```
// Enable/disable plugin
rankviewer_enabled 1

// Select playlist (11 = 2v2)
rankviewer_playlist 11

// Position
rankviewer_offset_x 700
rankviewer_offset_y -400

// Size
rankviewer_rect_width 470
rankviewer_rect_height 250

// Appearance
rankviewer_opacity 255
```

### Reset to Defaults

Click the "Reset to Default" button in the settings panel to restore all settings to their original values.

## Building from Source

### Requirements
- Visual Studio 2019 or later
- [BakkesMod Plugin SDK](https://github.com/bakkesmodorg/BakkesModSDK)
- C++17 or later

### Build Steps

1. **Clone the Repository**
```bash
   git clone https://github.com/YourUsername/RocketLeagueRankViewer.git
   cd RocketLeagueRankViewer
```

2. **Setup SDK**
   - Ensure BakkesMod SDK is properly installed
   - Update include paths in your project settings

3. **Build**
```bash
   # Open in Visual Studio
   # Build → Build Solution (Ctrl+Shift+B)
   # Or use MSBuild:
   msbuild RankViewerNVersion.sln /p:Configuration=Release
```

4. **Output**
   - Built DLL will be in `x64/Release/RankViewerNVersion.dll`

## Project Structure
```
RocketLeagueRankViewer/
├── src/
│   ├── RankViewerNVersion.cpp      # Main implementation
│   ├── RankViewerNVersion.h        # Header file
│   └── pch.h                       # Precompiled headers
├── assets/
│   ├── RankIcons/                  # Rank icon images
│   │   ├── 0.png - 22.png
│   └── RankNumbers/                # MMR threshold data
│       ├── 10.json - 34.json
├── docs/
│   ├── CHANGELOG.md
│   └── CONTRIBUTING.md
├── .gitignore
├── LICENSE
└── README.md
```

## Troubleshooting

### Plugin doesn't load
- Verify BakkesMod is installed and working
- Check BakkesMod console for error messages
- Ensure the DLL is in the correct plugins folder
- Try loading manually: `plugin load rankviewernversion`

### Rank not displaying
- Make sure you've played at least 10 placement matches in the selected playlist
- Check that the correct playlist is selected in settings
- Wait a few seconds after a match for MMR to sync
- Toggle the plugin off and on

### Icons not showing
- Verify the `RankViewer` folder is in the correct location
- Check that all PNG files are present in `RankIcons/`
- Ensure file names match exactly (0.png through 22.png)

### Performance issues
- Lower the opacity slightly
- Reduce the rectangle size
- Disable unused display options

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Development Guidelines
- Follow the existing code style
- Add comments for complex logic
- Test thoroughly before submitting
- Update documentation as needed

## Known Issues

- [ ] Rank icons may flicker briefly after match completion
- [ ] Tournament ranks may take longer to sync
- [ ] Very high MMR values (>2000) may display slightly off-center

See the [Issues](https://github.com/YourUsername/RocketLeagueRankViewer/issues) page for a full list.

## Roadmap

- [ ] Add rank history tracking
- [ ] Include win/loss streak indicators
- [ ] Add animated rank up/down notifications
- [ ] Support for custom themes
- [ ] Rank predictions based on current MMR
- [ ] Party member rank display

## Credits

- **Developer**: [Your Name](https://github.com/YourUsername)
- **BakkesMod**: [BakkesMod Team](https://www.bakkesmod.com/)
- **Rank Icons**: Psyonix/Rocket League
- **Contributors**: [See Contributors](https://github.com/YourUsername/RocketLeagueRankViewer/graphs/contributors)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Disclaimer

This is a third-party plugin and is not affiliated with or endorsed by Psyonix or Epic Games. Use at your own risk. While BakkesMod is generally safe to use, always ensure you're following the game's terms of service.

## Support

- **Issues**: [GitHub Issues](https://github.com/YourUsername/RocketLeagueRankViewer/issues)
- **Discord**: [BakkesMod Discord](https://discord.gg/bakkesmod)
- **Documentation**: [Wiki](https://github.com/YourUsername/RocketLeagueRankViewer/wiki)

---

**Star ⭐ this repository if you find it helpful!**

Made with ❤️ for the Rocket League community

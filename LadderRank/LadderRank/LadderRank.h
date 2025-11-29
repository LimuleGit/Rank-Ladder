#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "imgui/imgui.h"
#include "version.h"

#pragma comment(lib, "pluginsdk.lib")

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

using namespace std;

// ============================================================================
// MAIN PLUGIN CLASS
// ============================================================================

/**
 * @brief LadderRank Plugin
 *
 * Displays player rank information with customizable UI for Rocket League.
 * Shows current rank, MMR values, and adjacent rank thresholds.
 *
 * Inherits from:
 * - BakkesModPlugin: Core plugin functionality
 * - PluginWindow: ImGui window rendering
 * - SettingsWindowBase: Plugin settings interface
 */
class LadderRank : public BakkesMod::Plugin::BakkesModPlugin,
    public BakkesMod::Plugin::PluginWindow,
    public SettingsWindowBase
{
public:
    // ========================================================================
    // PLUGIN LIFECYCLE
    // ========================================================================

    void onLoad() override;
    void onUnload() override;

    // ========================================================================
    // SETTINGS INTERFACE
    // ========================================================================

    void RenderSettings() override;
    void Render() override;

    // ========================================================================
    // MENU INTERFACE
    // ========================================================================

    std::string GetMenuName() override;
    std::string GetMenuTitle() override;
    void SetImGuiContext(uintptr_t ctx) override;
    bool ShouldBlockInput() override;
    bool IsActiveOverlay() override;
    void OnOpen() override;
    void OnClose() override;

    // ========================================================================
    // PUBLIC MEMBERS
    // ========================================================================

    UniqueIDWrapper uniqueID;

    // ========================================================================
    // CORE FUNCTIONALITY
    // ========================================================================

    /**
     * @brief Checks and updates MMR data with retry mechanism
     * @param retryCount Number of retries remaining
     */
    void CheckMMR(int retryCount);

    /**
     * @brief Retrieves MMR threshold for a specific rank/division
     * @param mode Playlist ID
     * @param rank Tier level
     * @param div Division within tier
     * @param upperLimit True for max MMR, false for min MMR
     * @return MMR threshold value
     */
    int unranker(int mode, int rank, int div, bool upperLimit);

    /**
     * @brief Event handler for menu loading
     * @param eventName Name of the triggered event
     */
    void loadMenu(std::string eventName);

    /**
     * @brief Event handler for post-game stats screen
     * @param eventName Name of the triggered event
     */
    void StatsScreen(std::string eventName);

private:
    // ========================================================================
    // INITIALIZATION
    // ========================================================================

    /**
     * @brief Registers all CVars for plugin settings
     */
    void RegisterCVars();

    /**
     * @brief Registers game event hooks
     */
    void RegisterEventHooks();

    // ========================================================================
    // DATA LOADING
    // ========================================================================

    /**
     * @brief Loads default rank data for the selected playlist
     */
    void LoadDefaultRankData();

    /**
     * @brief Loads rank icon images based on current tier
     */
    void LoadRankIcons();

    // ========================================================================
    // MMR UPDATE SYSTEM
    // ========================================================================

    /**
     * @brief Validates game state before MMR check
     * @param retryCount Current retry attempt
     * @return True if game state is valid
     */
    bool IsValidGameState(int retryCount);

    /**
     * @brief Attempts to retrieve MMR data from wrapper
     * @param retryCount Number of retries remaining
     */
    void TryGetMMRData(int retryCount);

    /**
     * @brief Checks if playlist is a ranked playlist
     * @param playlist Playlist ID to check
     * @return True if ranked playlist
     */
    bool IsRankedPlaylist(int playlist);

    /**
     * @brief Fetches and updates player rank data
     * @param mmrWrapper Reference to MMR wrapper
     */
    void FetchPlayerRankData(MMRWrapper& mmrWrapper);

    // ========================================================================
    // RANK CALCULATION
    // ========================================================================

    /**
     * @brief Calculates adjacent ranks and MMR thresholds
     */
    void CalculateAdjacentRanks();

    /**
     * @brief Handles rank calculation for placement matches
     */
    void HandlePlacementMatches();

    /**
     * @brief Handles rank calculation for lowest rank (Bronze I Div I)
     */
    void HandleLowestRank();

    /**
     * @brief Handles rank calculation for highest rank (SSL)
     */
    void HandleHighestRank();

    /**
     * @brief Handles rank calculation for normal ranks
     */
    void HandleNormalRank();

    // ========================================================================
    // CANVAS RENDERING
    // ========================================================================

    /**
     * @brief Main canvas rendering function
     * @param canvas Canvas wrapper for drawing
     */
    void RenderCanvas(CanvasWrapper canvas);

    /**
     * @brief Renders left side of canvas (current rank display)
     * @param canvas Canvas wrapper reference
     * @param rectLeft Left edge of rectangle
     * @param rectTop Top edge of rectangle
     * @param rectBottom Bottom edge of rectangle
     * @param leftMargin Left margin value
     * @param xPercent Horizontal scaling factor
     * @param yPercent Vertical scaling factor
     */
    void RenderLeftSide(CanvasWrapper& canvas, float rectLeft, float rectTop,
        float rectBottom, float leftMargin, float xPercent, float yPercent);

    /**
     * @brief Renders right side of canvas (rank progression)
     * @param canvas Canvas wrapper reference
     * @param rectRight Right edge of rectangle
     * @param rectTop Top edge of rectangle
     * @param rectBottom Bottom edge of rectangle
     * @param rightIconOffset Icon offset from right edge
     * @param textOffset Text offset from icons
     * @param imgSize Icon size
     * @param topSpacing Top rank vertical spacing
     * @param middleSpacing Middle rank vertical spacing
     * @param bottomSpacing Bottom rank vertical spacing
     * @param xPercent Horizontal scaling factor
     * @param yPercent Vertical scaling factor
     */
    void RenderRightSide(CanvasWrapper& canvas, float rectRight, float rectTop,
        float rectBottom, float rightIconOffset, float textOffset,
        float imgSize, float topSpacing, float middleSpacing,
        float bottomSpacing, float xPercent, float yPercent);

    /**
     * @brief Renders a single rank entry (MMR + icon)
     * @param canvas Canvas wrapper reference
     * @param textX X position for text
     * @param iconX X position for icon
     * @param textY Y position for text
     * @param iconY Y position for icon
     * @param mmrValue MMR value to display
     * @param rankIcon Rank icon to render
     * @param imgSize Icon size
     * @param xPercent Horizontal scaling factor
     * @param yPercent Vertical scaling factor
     */
    void RenderRankEntry(CanvasWrapper& canvas, float textX, float iconX,
        float textY, float iconY, int mmrValue,
        std::shared_ptr<ImageWrapper> rankIcon,
        float imgSize, float xPercent, float yPercent);

    // ========================================================================
    // SETTINGS UI RENDERING
    // ========================================================================

    /**
     * @brief Renders playlist selection dropdown
     */
    void RenderPlaylistSelector();

    /**
     * @brief Renders global position settings
     */
    void RenderPositionSettings();

    /**
     * @brief Renders rectangle dimension settings
     */
    void RenderRectangleSettings();

    /**
     * @brief Renders left side layout settings
     */
    void RenderLeftSideSettings();

    /**
     * @brief Renders right side layout settings
     */
    void RenderRightSideSettings();

    /**
     * @brief Renders vertical spacing settings
     */
    void RenderSpacingSettings();

    /**
     * @brief Renders customization settings (opacity, etc.)
     */
    void RenderCustomizationSettings();

    /**
     * @brief Resets all settings to default values
     */
    void ResetToDefaults();

    // ========================================================================
    // STATE VARIABLES
    // ========================================================================

    // Window state
    bool isWindowOpen_ = false;
    bool isOpen = false;
    Vector2 screenSize;

    // Display flags
    bool shouldDraw = true;
    bool isEnabled = false;
    bool gotNewMMR = false;
    bool drawCanvas = false;
    bool isFriendOpen = false;

    // Visibility toggles
    bool rankNext = true;      // Show next rank
    bool rankUnder = true;     // Show previous rank
    bool rankAverage = true;   // Show current rank (right side)
    bool rankAverage2 = true;  // Show current rank (left side)

    // ========================================================================
    // RANK DATA
    // ========================================================================

    // Current rank information
    int userTier = 0;      // Current tier (0-22)
    int userDiv = 0;       // Current division (0-3)
    int userPlaylist = 0;  // Selected playlist ID
    float userMMR = 0.0f;  // Current MMR value

    // Adjacent rank information (for division thresholds)
    int upperTier = 0;     // Tier above current
    int lowerTier = 0;     // Tier below current
    int upperDiv = 0;      // Division above current
    int lowerDiv = 0;      // Division below current

    // MMR thresholds for divisions
    int nextLower = 0;     // MMR for next division up
    int beforeUpper = 0;   // MMR for previous division down

    // MMR thresholds for complete tiers (for display)
    int nextTierMinMMR = 0;  // Minimum MMR for tier +1
    int prevTierMaxMMR = 0;  // Minimum MMR for current tier

    // Rank display names
    std::string nameCurrent = "";  // Current rank name
    std::string nameNext = "";     // Next rank name
    std::string nameBefore = "";   // Previous rank name

    // ========================================================================
    // VISUAL ASSETS
    // ========================================================================

    std::shared_ptr<ImageWrapper> currentRank;  // Current rank icon
    std::shared_ptr<ImageWrapper> nextRank;     // Next rank icon
    std::shared_ptr<ImageWrapper> beforeRank;   // Previous rank icon

    // Visual settings
    float opacity = 255.0f;  // Background opacity (0-255)

    // ========================================================================
    // CONSTANTS
    // ========================================================================

    /**
     * @brief Array of ranked playlist IDs
     */
    static constexpr int rankedPlaylists[8] = {
        10,  // 1v1 (Duel)
        11,  // 2v2 (Doubles)
        13,  // 3v3 (Standard)
        27,  // Hoops
        28,  // Rumble
        29,  // Dropshot
        30,  // Snowday
        34   // Tournaments
    };

    // ========================================================================
    // EVENT HANDLING
    // ========================================================================

    /**
     * @brief FName structure for Unreal Engine event handling
     */
    struct FName2 {
        int32_t Index;
        int32_t Instance;
    };
};
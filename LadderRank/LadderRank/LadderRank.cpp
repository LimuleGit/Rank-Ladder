#include "pch.h"
#include "LadderRank.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"
#include "utils/parser.h"
#include "json.hpp"

using json = nlohmann::json;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
BAKKESMOD_PLUGIN(LadderRank, "LadderRank", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Converts rank tier and division to display name
 * @param rank Tier level (0-22)
 * @param div Division within tier (0-3)
 * @return Display name like "DIV I", "DIV II", etc.
 */
std::string GetDivName(int rank, int div) {
    if (rank < 0 || rank > 22) {
        return "ERROR";
    }

    if (rank == 0 || rank == 22) {
        return " ";
    }

    const std::string divNumbers[] = { "I", "II", "III", "IV" };
    return "DIV " + divNumbers[div];
}

// ============================================================================
// PLUGIN LIFECYCLE
// ============================================================================

void LadderRank::onLoad() {
    _globalCvarManager = cvarManager;
    LOG("Plugin loaded!");

    // Initialize screen size
    screenSize = gameWrapper->GetScreenSize();
    LOG("Screen size: {}x{}", screenSize.X, screenSize.Y);

    // Register rendering callback
    gameWrapper->RegisterDrawable(std::bind(&LadderRank::RenderCanvas, this, std::placeholders::_1));

    // Register CVars
    RegisterCVars();

    // Initialize state
    shouldDraw = true;
    drawCanvas = true;

    // Load rank data after delay
    gameWrapper->SetTimeout([this](GameWrapper* gw) {
        LoadDefaultRankData();
        }, 2.0f);

    // Hook game events
    RegisterEventHooks();

    // Initialize rank images with default
    auto defaultPath = gameWrapper->GetDataFolder() / "LadderRank" / "RankIcons" / "0.png";
    currentRank = std::make_shared<ImageWrapper>(defaultPath, true, false);
    nextRank = std::make_shared<ImageWrapper>(defaultPath, true, false);
    beforeRank = std::make_shared<ImageWrapper>(defaultPath, true, false);

    LOG("Plugin initialization complete");
}

void LadderRank::onUnload() {
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");
    gameWrapper->UnhookEvent("Function TAGame.GFxData_MenuStack_TA.ButtonTriggered");
    gameWrapper->UnregisterDrawables();
}

// ============================================================================
// CVAR AND EVENT REGISTRATION
// ============================================================================

void LadderRank::RegisterCVars() {
    // Plugin state
    cvarManager->registerCvar("LadderRank_enabled", "1",
        "Enable or Disable the Rank Viewer Plugin", true, true, 0, true, 1, true);

    cvarManager->registerCvar("LadderRank_playlist", "11",
        "Playlist to display (10=1v1, 11=2v2, 13=3v3, 27=Hoops, 28=Rumble, 29=Dropshot, 30=Snowday)",
        true, true, 10, true, 34);

    // Global position
    cvarManager->registerCvar("LadderRank_offset_x", "700",
        "Horizontal offset for entire canvas", true, true, -1000, true, 1000);
    cvarManager->registerCvar("LadderRank_offset_y", "-400",
        "Vertical offset for entire canvas", true, true, -1000, true, 1000);

    // Rectangle dimensions
    cvarManager->registerCvar("LadderRank_rect_width", "470",
        "Width of the rectangle", true, true, 50, true, 800);
    cvarManager->registerCvar("LadderRank_rect_height", "250",
        "Height of the rectangle", true, true, 150, true, 1000);

    // Layout settings
    cvarManager->registerCvar("LadderRank_left_margin", "30",
        "Left margin for text", true, true, 0, true, 200);
    cvarManager->registerCvar("LadderRank_icon_size", "60",
        "Size of rank icons", true, true, 30, true, 150);
    cvarManager->registerCvar("LadderRank_right_icon_offset", "80",
        "Distance of right icons from edge", true, true, 50, true, 200);
    cvarManager->registerCvar("LadderRank_text_offset", "150",
        "Distance of text from right icons", true, true, 50, true, 300);

    // Vertical spacing
    cvarManager->registerCvar("LadderRank_top_spacing", "120",
        "Vertical spacing for top rank", true, true, 50, true, 200);
    cvarManager->registerCvar("LadderRank_middle_spacing", "30",
        "Vertical spacing for middle rank", true, true, 0, true, 100);
    cvarManager->registerCvar("LadderRank_bottom_spacing", "60",
        "Vertical spacing for bottom rank", true, true, 30, true, 150);

    // Visual settings
    cvarManager->registerCvar("LadderRank_opacity", "255",
        "Adjust the opacity of the background", true, true, 0.f, true, 255.f);
}

void LadderRank::RegisterEventHooks() {
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet",
        std::bind(&LadderRank::StatsScreen, this, std::placeholders::_1));

    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed",
        std::bind(&LadderRank::loadMenu, this, std::placeholders::_1));
}

// ============================================================================
// RANK DATA LOADING
// ============================================================================

void LadderRank::LoadDefaultRankData() {
    LOG("LoadDefaultRankData called");

    MMRWrapper mmrWrapper = gameWrapper->GetMMRWrapper();
    uniqueID = gameWrapper->GetUniqueID();
    LOG("UniqueID retrieved");

    // Get selected playlist
    userPlaylist = cvarManager->getCvar("LadderRank_playlist").getIntValue();

    // Check sync status
    bool isSynced = mmrWrapper.IsSynced(uniqueID, userPlaylist);
    bool isSyncing = mmrWrapper.IsSyncing(uniqueID);
    LOG("IsSynced: {}, IsSyncing: {}", isSynced, isSyncing);

    // Wait for sync if needed
    if (!isSynced || isSyncing) {
        LOG("MMR data not synced yet, will retry in 1 second");
        nameCurrent = "Loading...";
        userMMR = 0.0f;

        gameWrapper->SetTimeout([this](GameWrapper* gw) {
            LoadDefaultRankData();
            }, 1.0f);
        return;
    }

    // Load player data
    userMMR = mmrWrapper.GetPlayerMMR(uniqueID, userPlaylist);
    LOG("UserMMR: {}", userMMR);

    SkillRank userRank = mmrWrapper.GetPlayerRank(uniqueID, userPlaylist);
    userDiv = userRank.Division;
    userTier = userRank.Tier;
    LOG("UserTier: {}, UserDiv: {}", userTier, userDiv);

    nameCurrent = GetDivName(userTier, userDiv);
    LOG("Rank name: {}", nameCurrent);

    CalculateAdjacentRanks();
    LoadRankIcons();

    LOG("Loaded playlist {} rank data successfully: Tier={}, Div={}, MMR={}",
        userPlaylist, userTier, userDiv, userMMR);
}

void LadderRank::LoadRankIcons() {
    auto dataFolder = gameWrapper->GetDataFolder() / "LadderRank" / "RankIcons";

    // Current rank icon (middle)
    auto currentPath = dataFolder / (std::to_string(userTier) + ".png");
    currentRank = std::make_shared<ImageWrapper>(currentPath, true, false);
    LOG("Loading CURRENT rank icon: {} (tier={})", currentPath.string(), userTier);

    // Next rank icon (top) - always show tier +1
    int visualUpperTier = (userTier >= 22) ? 22 : userTier + 1;
    auto nextPath = dataFolder / (std::to_string(visualUpperTier) + ".png");
    nextRank = std::make_shared<ImageWrapper>(nextPath, true, false);
    LOG("Loading NEXT rank icon: {} (tier={})", nextPath.string(), visualUpperTier);

    // Previous rank icon (bottom) - always show tier -1
    int visualLowerTier = (userTier <= 1) ? 1 : userTier - 1;
    auto beforePath = dataFolder / (std::to_string(visualLowerTier) + ".png");
    beforeRank = std::make_shared<ImageWrapper>(beforePath, true, false);
    LOG("Loading BEFORE rank icon: {} (tier={})", beforePath.string(), visualLowerTier);

    LOG("Loaded 3 rank icons: before={}, current={}, next={}",
        visualLowerTier, userTier, visualUpperTier);
}

// ============================================================================
// MMR CALCULATION
// ============================================================================

int LadderRank::unranker(int mode, int rank, int div, bool upperLimit) {
    std::string fileName = std::to_string(mode) + ".json";
    const auto rankJSON = gameWrapper->GetDataFolder() / "LadderRank" / "RankNumbers" / fileName;

    std::string limit = upperLimit ? "maxMMR" : "minMMR";

    std::ifstream file(rankJSON);
    json j = json::parse(file);

    return j["data"]["data"][((rank - 1) * 4) + (div + 1)][limit];
}

void LadderRank::CalculateAdjacentRanks() {
    // Calculate adjacent ranks based on division (for division thresholds)
    if (userTier <= 0) {
        HandlePlacementMatches();
    }
    else if (userTier == 1 && userDiv == 0) {
        HandleLowestRank();
    }
    else if (userTier == 22) {
        HandleHighestRank();
    }
    else {
        HandleNormalRank();
    }

    // Calculate MMR for complete tiers (±1 tier) for display
    int nextTierForDisplay = (userTier >= 22) ? 22 : userTier + 1;
    int prevTierForDisplay = (userTier <= 1) ? 1 : userTier - 1;

    // Minimum MMR of next tier (always Div I)
    nextTierMinMMR = unranker(userPlaylist, nextTierForDisplay, 0, false);

    // Minimum MMR of current tier (falling below this drops you to lower tier)
    prevTierMaxMMR = unranker(userPlaylist, userTier, 0, false);

    LOG("Adjacent ranks: lower={}(div {}), current={}(div {}), upper={}(div {})",
        lowerTier, lowerDiv, userTier, userDiv, upperTier, upperDiv);
    LOG("Division MMR thresholds: beforeUpper={}, nextLower={}", beforeUpper, nextLower);
    LOG("Tier MMR display: currentTierMin={}, nextTierMin={}", prevTierMaxMMR, nextTierMinMMR);
}

void LadderRank::HandlePlacementMatches() {
    lowerTier = 1;
    upperTier = 22;

    nextLower = unranker(userPlaylist, upperTier, 0, true);
    nameNext = GetDivName(22, 0);

    beforeUpper = unranker(userPlaylist, lowerTier, 0, false);
    nameBefore = GetDivName(0, 0);
}

void LadderRank::HandleLowestRank() {
    upperTier = userTier;
    lowerTier = userTier;
    upperDiv = userDiv + 1;
    lowerDiv = 0;

    nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
    nameNext = GetDivName(upperTier, upperDiv);

    beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, false);
    nameBefore = GetDivName(lowerTier, lowerDiv);
}

void LadderRank::HandleHighestRank() {
    upperTier = userTier;
    lowerTier = userTier - 1;
    upperDiv = 0;
    lowerDiv = 3;

    nextLower = unranker(userPlaylist, userTier, 0, true);
    nameNext = GetDivName(userTier, 0);

    beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
    nameBefore = GetDivName(lowerTier, lowerDiv);
}

void LadderRank::HandleNormalRank() {
    if (userDiv == 0) {
        // First division of a rank
        upperTier = userTier;
        lowerTier = userTier - 1;
        upperDiv = userDiv + 1;
        lowerDiv = 3;
    }
    else if (userDiv == 3) {
        // Last division of a rank
        upperTier = userTier + 1;
        lowerTier = userTier;
        upperDiv = 0;
        lowerDiv = userDiv - 1;
    }
    else {
        // Middle divisions
        upperTier = userTier;
        lowerTier = userTier;
        upperDiv = userDiv + 1;
        lowerDiv = userDiv - 1;
    }

    nextLower = unranker(userPlaylist, upperTier, upperDiv, false);
    nameNext = GetDivName(upperTier, upperDiv);

    beforeUpper = unranker(userPlaylist, lowerTier, lowerDiv, true);
    nameBefore = GetDivName(lowerTier, lowerDiv);
}

// ============================================================================
// RENDERING
// ============================================================================

void LadderRank::RenderCanvas(CanvasWrapper canvas) {
    if (!drawCanvas || !shouldDraw) {
        return;
    }

    // Calculate scaling factors
    float xPercent = static_cast<float>(screenSize.X) / 1920.0f;
    float yPercent = static_cast<float>(screenSize.Y) / 1080.0f;

    // Get layout parameters
    float offsetX = cvarManager->getCvar("LadderRank_offset_x").getFloatValue() * xPercent;
    float offsetY = cvarManager->getCvar("LadderRank_offset_y").getFloatValue() * yPercent;
    float rectWidth = cvarManager->getCvar("LadderRank_rect_width").getFloatValue();
    float rectHeight = cvarManager->getCvar("LadderRank_rect_height").getFloatValue();
    float leftMargin = cvarManager->getCvar("LadderRank_left_margin").getFloatValue();
    float imgSize = cvarManager->getCvar("LadderRank_icon_size").getFloatValue() * yPercent;
    float rightIconOffset = cvarManager->getCvar("LadderRank_right_icon_offset").getFloatValue();
    float textOffset = cvarManager->getCvar("LadderRank_text_offset").getFloatValue();
    float topSpacing = cvarManager->getCvar("LadderRank_top_spacing").getFloatValue();
    float middleSpacing = cvarManager->getCvar("LadderRank_middle_spacing").getFloatValue();
    float bottomSpacing = cvarManager->getCvar("LadderRank_bottom_spacing").getFloatValue();

    // Calculate rectangle position
    float rectLeft = ((screenSize.X - (rectWidth * xPercent)) / 2) + offsetX;
    float rectTop = ((screenSize.Y - (rectHeight * yPercent)) / 2) + offsetY;
    float rectRight = rectLeft + (rectWidth * xPercent);
    float rectBottom = rectTop + (rectHeight * yPercent);

    // Draw background rectangle
    canvas.SetColor(0, 0, 0, opacity);
    canvas.SetPosition(Vector2{ static_cast<int>(rectLeft), static_cast<int>(rectTop) });
    canvas.FillBox(Vector2{ static_cast<int>(rectRight - rectLeft), static_cast<int>(rectBottom - rectTop) });

    // Draw left side (current rank info)
    RenderLeftSide(canvas, rectLeft, rectTop, rectBottom, leftMargin, xPercent, yPercent);

    // Draw right side (rank progression)
    RenderRightSide(canvas, rectRight, rectTop, rectBottom, rightIconOffset, textOffset,
        imgSize, topSpacing, middleSpacing, bottomSpacing, xPercent, yPercent);
}

void LadderRank::RenderLeftSide(CanvasWrapper& canvas, float rectLeft, float rectTop,
    float rectBottom, float leftMargin, float xPercent, float yPercent) {
    if (!rankAverage2) {
        return;
    }

    float textLeftX = rectLeft + (leftMargin * xPercent);
    float centerY = (rectTop + rectBottom) / 2;

    canvas.SetColor(255, 255, 255, 255);

    // Draw "Rank :" label and current rank icon
    canvas.SetPosition(Vector2{ static_cast<int>(textLeftX), static_cast<int>(centerY - (30 * yPercent)) });
    canvas.DrawString("Rank :", 2.0f, 2.0f);

    if (currentRank->IsLoadedForCanvas()) {
        float iconSize = 40.0f * yPercent;
        float iconX = textLeftX + 100 * xPercent;
        float iconY = centerY - (30 * yPercent) - 5;
        canvas.SetPosition(Vector2{ static_cast<int>(iconX), static_cast<int>(iconY) });
        canvas.DrawTexture(currentRank.get(), iconSize / currentRank->GetSize().Y);
    }

    // Draw "MMR :" label and current MMR value
    canvas.SetPosition(Vector2{ static_cast<int>(textLeftX), static_cast<int>(centerY + (10 * yPercent)) });
    std::string mmrText = "MMR : " + std::to_string(static_cast<int>(userMMR));
    canvas.DrawString(mmrText, 2.0f, 2.0f);
}

void LadderRank::RenderRightSide(CanvasWrapper& canvas, float rectRight, float rectTop,
    float rectBottom, float rightIconOffset, float textOffset,
    float imgSize, float topSpacing, float middleSpacing,
    float bottomSpacing, float xPercent, float yPercent) {
    float iconsRightX = rectRight - (rightIconOffset * xPercent);
    float iconsCenterY = (rectTop + rectBottom) / 2;
    float textRightX = iconsRightX - (textOffset * xPercent);

    canvas.SetColor(255, 255, 255, 255);

    // Render next rank (top)
    if (rankNext) {
        RenderRankEntry(canvas, textRightX, iconsRightX, iconsCenterY - (topSpacing * yPercent),
            iconsCenterY - ((topSpacing - 20) * yPercent), nextTierMinMMR,
            nextRank, imgSize, xPercent, yPercent);
    }

    // Render current rank (middle)
    if (rankAverage) {
        RenderRankEntry(canvas, textRightX, iconsRightX, iconsCenterY - (middleSpacing * yPercent),
            iconsCenterY - (10 * yPercent), static_cast<int>(userMMR),
            currentRank, imgSize, xPercent, yPercent);
    }

    // Render previous rank (bottom)
    if (rankUnder) {
        RenderRankEntry(canvas, textRightX, iconsRightX, iconsCenterY + (40 * yPercent),
            iconsCenterY + (bottomSpacing * yPercent), prevTierMaxMMR,
            beforeRank, imgSize, xPercent, yPercent);
    }
}

void LadderRank::RenderRankEntry(CanvasWrapper& canvas, float textX, float iconX,
    float textY, float iconY, int mmrValue,
    std::shared_ptr<ImageWrapper> rankIcon,
    float imgSize, float xPercent, float yPercent) {
    // Draw MMR label
    canvas.SetPosition(Vector2{ static_cast<int>(textX), static_cast<int>(textY) });
    canvas.DrawString("MMR :", 1.5f, 1.5f);

    // Draw MMR value
    canvas.SetPosition(Vector2{ static_cast<int>(textX + 80 * xPercent), static_cast<int>(textY) });
    canvas.DrawString(std::to_string(mmrValue), 1.5f, 1.5f);

    // Draw Rank label
    canvas.SetPosition(Vector2{ static_cast<int>(textX), static_cast<int>(textY + 30 * yPercent) });
    canvas.DrawString("Rank :", 1.5f, 1.5f);

    // Draw rank icon
    if (rankIcon->IsLoadedForCanvas()) {
        canvas.SetPosition(Vector2{ static_cast<int>(iconX), static_cast<int>(iconY) });
        canvas.DrawTexture(rankIcon.get(), imgSize / rankIcon->GetSize().Y);
    }
}

// ============================================================================
// MMR UPDATE SYSTEM
// ============================================================================

void LadderRank::CheckMMR(int retryCount) {
    if (!IsValidGameState(retryCount)) {
        return;
    }

    if (userPlaylist == 0) {
        return;
    }

    gameWrapper->SetTimeout([retryCount, this](GameWrapper* gw) {
        TryGetMMRData(retryCount);
        }, 3.0f);
}

bool LadderRank::IsValidGameState(int retryCount) {
    isEnabled = cvarManager->getCvar("LadderRank_enabled").getBoolValue();
    if (!isEnabled) {
        return false;
    }

    ServerWrapper server = gameWrapper->GetOnlineGame();
    if (server.IsNull() || !server.IsOnlineMultiplayer() || gameWrapper->IsInReplay()) {
        return false;
    }

    if (retryCount < 0 || retryCount > 20) {
        return false;
    }

    return true;
}

void LadderRank::TryGetMMRData(int retryCount) {
    MMRWrapper mmrWrapper = gameWrapper->GetMMRWrapper();

    if (mmrWrapper.IsSynced(uniqueID, userPlaylist) && !mmrWrapper.IsSyncing(uniqueID)) {
        if (!IsRankedPlaylist(userPlaylist)) {
            LOG("Not a ranked playlist: {}", userPlaylist);
            return;
        }

        FetchPlayerRankData(mmrWrapper);
        LoadRankIcons();

        drawCanvas = true;
        gotNewMMR = true;
    }
    else if (retryCount > 0) {
        gameWrapper->SetTimeout([retryCount, this](GameWrapper* gw) {
            this->CheckMMR(retryCount - 1);
            }, 0.5f);
    }
}

bool LadderRank::IsRankedPlaylist(int playlist) {
    return std::find(std::begin(rankedPlaylists), std::end(rankedPlaylists), playlist)
        != std::end(rankedPlaylists);
}

void LadderRank::FetchPlayerRankData(MMRWrapper& mmrWrapper) {
    userMMR = mmrWrapper.GetPlayerMMR(uniqueID, userPlaylist);

    SkillRank userRank = mmrWrapper.GetPlayerRank(uniqueID, userPlaylist);
    userDiv = userRank.Division;
    userTier = userRank.Tier;

    nameCurrent = GetDivName(userTier, userDiv);

    CalculateAdjacentRanks();
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void LadderRank::StatsScreen(std::string eventName) {
    isEnabled = cvarManager->getCvar("LadderRank_enabled").getBoolValue();
    if (!isEnabled) {
        return;
    }

    MMRWrapper mw = gameWrapper->GetMMRWrapper();
    uniqueID = gameWrapper->GetUniqueID();
    userPlaylist = mw.GetCurrentPlaylist();
    screenSize = gameWrapper->GetScreenSize();
    isFriendOpen = false;

    if (mw.IsRanked(userPlaylist)) {
        CheckMMR(5);
    }
}

void LadderRank::loadMenu(std::string eventName) {
    drawCanvas = false;
    isFriendOpen = false;
}

// ============================================================================
// SETTINGS UI
// ============================================================================

void LadderRank::RenderSettings() {
    ImGui::TextUnformatted("Rocket League Rank Viewer");

    ImGui::Checkbox("Enable Display", &shouldDraw);
    ImGui::Checkbox("Show Next Rank", &rankNext);
    ImGui::Checkbox("Show Previous Rank", &rankUnder);
    ImGui::Checkbox("Show Current Rank (Right)", &rankAverage);
    ImGui::Checkbox("Show Current Rank (Left)", &rankAverage2);

    ImGui::Separator();
    RenderPlaylistSelector();

    ImGui::Separator();
    RenderPositionSettings();

    ImGui::Separator();
    RenderRectangleSettings();

    ImGui::Separator();
    RenderLeftSideSettings();

    ImGui::Separator();
    RenderRightSideSettings();

    ImGui::Separator();
    RenderSpacingSettings();

    ImGui::Separator();
    RenderCustomizationSettings();

    ImGui::Separator();
    if (ImGui::Button("Reset to Default")) {
        ResetToDefaults();
    }
}

void LadderRank::RenderPlaylistSelector() {
    ImGui::TextUnformatted("Rank Selection");

    CVarWrapper playlistCvar = cvarManager->getCvar("LadderRank_playlist");
    if (!playlistCvar) {
        return;
    }

    int currentPlaylist = playlistCvar.getIntValue();

    const char* playlistNames[] = { "1v1", "2v2", "3v3", "Hoops", "Rumble", "Dropshot", "Snowday", "Tournament" };
    const int playlistValues[] = { 10, 11, 13, 27, 28, 29, 30, 34 };

    // Find current index
    int currentIndex = 1; // Default to 2v2
    for (int i = 0; i < 8; i++) {
        if (playlistValues[i] == currentPlaylist) {
            currentIndex = i;
            break;
        }
    }

    if (ImGui::Combo("Playlist", &currentIndex, playlistNames, 8)) {
        playlistCvar.setValue(playlistValues[currentIndex]);
        gameWrapper->SetTimeout([this](GameWrapper* gw) {
            LoadDefaultRankData();
            }, 0.1f);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Choose which playlist rank to display");
    }
}

void LadderRank::RenderPositionSettings() {
    ImGui::TextUnformatted("Global Position");

    CVarWrapper offsetXCvar = cvarManager->getCvar("LadderRank_offset_x");
    if (offsetXCvar) {
        float offsetX = offsetXCvar.getFloatValue();
        if (ImGui::SliderFloat("Horizontal Offset", &offsetX, -1000.0f, 1000.0f)) {
            offsetXCvar.setValue(offsetX);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Move entire canvas left/right");
        }
    }

    CVarWrapper offsetYCvar = cvarManager->getCvar("LadderRank_offset_y");
    if (offsetYCvar) {
        float offsetY = offsetYCvar.getFloatValue();
        if (ImGui::SliderFloat("Vertical Offset", &offsetY, -1000.0f, 1000.0f)) {
            offsetYCvar.setValue(offsetY);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Move entire canvas up/down");
        }
    }
}

void LadderRank::RenderRectangleSettings() {
    ImGui::TextUnformatted("Rectangle Settings");

    CVarWrapper rectWidthCvar = cvarManager->getCvar("LadderRank_rect_width");
    if (rectWidthCvar) {
        float rectWidth = rectWidthCvar.getFloatValue();
        if (ImGui::SliderFloat("Rectangle Width", &rectWidth, 50.0f, 800.0f)) {
            rectWidthCvar.setValue(rectWidth);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Width of the main rectangle");
        }
    }

    CVarWrapper rectHeightCvar = cvarManager->getCvar("LadderRank_rect_height");
    if (rectHeightCvar) {
        float rectHeight = rectHeightCvar.getFloatValue();
        if (ImGui::SliderFloat("Rectangle Height", &rectHeight, 150.0f, 1000.0f)) {
            rectHeightCvar.setValue(rectHeight);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Height of the main rectangle");
        }
    }
}

void LadderRank::RenderLeftSideSettings() {
    ImGui::TextUnformatted("Left Side Settings");

    CVarWrapper leftMarginCvar = cvarManager->getCvar("LadderRank_left_margin");
    if (leftMarginCvar) {
        float leftMargin = leftMarginCvar.getFloatValue();
        if (ImGui::SliderFloat("Left Margin", &leftMargin, 0.0f, 200.0f)) {
            leftMarginCvar.setValue(leftMargin);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Distance from left edge to text");
        }
    }
}

void LadderRank::RenderRightSideSettings() {
    ImGui::TextUnformatted("Right Side Settings");

    CVarWrapper iconSizeCvar = cvarManager->getCvar("LadderRank_icon_size");
    if (iconSizeCvar) {
        float iconSize = iconSizeCvar.getFloatValue();
        if (ImGui::SliderFloat("Icon Size", &iconSize, 30.0f, 150.0f)) {
            iconSizeCvar.setValue(iconSize);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Size of rank icons on the right");
        }
    }

    CVarWrapper rightIconOffsetCvar = cvarManager->getCvar("LadderRank_right_icon_offset");
    if (rightIconOffsetCvar) {
        float rightIconOffset = rightIconOffsetCvar.getFloatValue();
        if (ImGui::SliderFloat("Icon Offset From Right", &rightIconOffset, 50.0f, 200.0f)) {
            rightIconOffsetCvar.setValue(rightIconOffset);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Distance of icons from right edge");
        }
    }

    CVarWrapper textOffsetCvar = cvarManager->getCvar("LadderRank_text_offset");
    if (textOffsetCvar) {
        float textOffset = textOffsetCvar.getFloatValue();
        if (ImGui::SliderFloat("Text Offset", &textOffset, 50.0f, 300.0f)) {
            textOffsetCvar.setValue(textOffset);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Distance of text from icons");
        }
    }
}

void LadderRank::RenderSpacingSettings() {
    ImGui::TextUnformatted("Vertical Spacing");

    CVarWrapper topSpacingCvar = cvarManager->getCvar("LadderRank_top_spacing");
    if (topSpacingCvar) {
        float topSpacing = topSpacingCvar.getFloatValue();
        if (ImGui::SliderFloat("Top Rank Spacing", &topSpacing, 50.0f, 200.0f)) {
            topSpacingCvar.setValue(topSpacing);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Vertical position of top rank");
        }
    }

    CVarWrapper middleSpacingCvar = cvarManager->getCvar("LadderRank_middle_spacing");
    if (middleSpacingCvar) {
        float middleSpacing = middleSpacingCvar.getFloatValue();
        if (ImGui::SliderFloat("Middle Rank Spacing", &middleSpacing, 0.0f, 100.0f)) {
            middleSpacingCvar.setValue(middleSpacing);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Vertical position of middle rank");
        }
    }

    CVarWrapper bottomSpacingCvar = cvarManager->getCvar("LadderRank_bottom_spacing");
    if (bottomSpacingCvar) {
        float bottomSpacing = bottomSpacingCvar.getFloatValue();
        if (ImGui::SliderFloat("Bottom Rank Spacing", &bottomSpacing, 30.0f, 150.0f)) {
            bottomSpacingCvar.setValue(bottomSpacing);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Vertical position of bottom rank");
        }
    }
}

void LadderRank::RenderCustomizationSettings() {
    ImGui::TextUnformatted("Customization");

    CVarWrapper opacityCvar = cvarManager->getCvar("LadderRank_opacity");
    if (opacityCvar) {
        opacity = opacityCvar.getFloatValue();
        if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 255.0f)) {
            opacityCvar.setValue(opacity);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Background opacity (0 = transparent, 255 = opaque)");
        }
    }
}

void LadderRank::ResetToDefaults() {
    cvarManager->getCvar("LadderRank_offset_x").setValue(700.0f);
    cvarManager->getCvar("LadderRank_offset_y").setValue(-400.0f);
    cvarManager->getCvar("LadderRank_rect_width").setValue(470.0f);
    cvarManager->getCvar("LadderRank_rect_height").setValue(250.0f);
    cvarManager->getCvar("LadderRank_left_margin").setValue(30.0f);
    cvarManager->getCvar("LadderRank_icon_size").setValue(60.0f);
    cvarManager->getCvar("LadderRank_right_icon_offset").setValue(80.0f);
    cvarManager->getCvar("LadderRank_text_offset").setValue(150.0f);
    cvarManager->getCvar("LadderRank_top_spacing").setValue(120.0f);
    cvarManager->getCvar("LadderRank_middle_spacing").setValue(30.0f);
    cvarManager->getCvar("LadderRank_bottom_spacing").setValue(60.0f);
    cvarManager->getCvar("LadderRank_opacity").setValue(255.0f);
}

// ============================================================================
// IMGUI RENDER (DEBUG/OVERLAY)
// ============================================================================

void LadderRank::Render() {
    float xPercent = static_cast<float>(screenSize.X) / 1920.0f;
    float yPercent = static_cast<float>(screenSize.Y) / 1080.0f;

    ImVec2 windowPos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(screenSize.X), static_cast<float>(screenSize.Y)));

    if (!ImGui::Begin(GetMenuTitle().c_str(), &isWindowOpen_,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Calculate rectangle dimensions
    const float rectWidth = 470.0f;
    const float rectHeight = 250.0f;

    float rectLeft = (screenSize.X - (rectWidth * xPercent)) / 2.0f;
    float rectTop = (screenSize.Y - (rectHeight * yPercent)) / 2.0f;
    float rectRight = rectLeft + (rectWidth * xPercent);
    float rectBottom = rectTop + (rectHeight * yPercent);

    // Draw background rectangle
    drawList->AddRectFilled(
        ImVec2(rectLeft, rectTop),
        ImVec2(rectRight, rectBottom),
        IM_COL32(0, 0, 0, 255)
    );

    // Draw text labels
    float textX = rectLeft + (30.0f * xPercent);
    float centerY = (rectTop + rectBottom) / 2.0f;

    float rankY = centerY - (30.0f * yPercent);
    drawList->AddText(
        ImVec2(textX, rankY),
        IM_COL32(255, 255, 255, 255),
        "Rank :"
    );

    float mmrY = centerY + (10.0f * yPercent);
    drawList->AddText(
        ImVec2(textX, mmrY),
        IM_COL32(255, 255, 255, 255),
        "MMR :"
    );

    // Draw rank icons
    ImVec2 centerPoint = ImVec2(screenSize.X / 2.0f, screenSize.Y / 2.0f);

    // Before rank icon (bottom)
    if (beforeRank->IsLoadedForImGui()) {
        if (auto beforeRankTex = beforeRank->GetImGuiTex()) {
            auto beforeRankRect = beforeRank->GetSizeF();
            ImGui::SetCursorPos(ImVec2(
                centerPoint.x - (beforeRankRect.X * 0.19f * xPercent) / 2.0f,
                centerPoint.y - (50.0f * yPercent) - (beforeRankRect.Y * 0.19f * yPercent) / 2.0f
            ));
            ImGui::Image(beforeRankTex, ImVec2(
                beforeRankRect.X * 0.19f * xPercent,
                beforeRankRect.Y * 0.19f * yPercent
            ));
        }
    }

    // Next rank icon (top)
    if (nextRank->IsLoadedForImGui()) {
        if (auto nextRankTex = nextRank->GetImGuiTex()) {
            auto nextRankRect = nextRank->GetSizeF();
            ImGui::SetCursorPos(ImVec2(
                centerPoint.x - (nextRankRect.X * 0.19f * xPercent) / 2.0f,
                centerPoint.y - (25.0f * yPercent) - (nextRankRect.Y * 0.19f * yPercent) / 2.0f
            ));
            ImGui::Image(nextRankTex, ImVec2(
                nextRankRect.X * 0.19f * xPercent,
                nextRankRect.Y * 0.19f * yPercent
            ));
        }
    }

    // Current rank icon (middle)
    if (currentRank->IsLoadedForImGui()) {
        if (auto currentRankTex = currentRank->GetImGuiTex()) {
            auto currentRankRect = currentRank->GetSizeF();
            ImGui::SetCursorPos(ImVec2(
                centerPoint.x - (currentRankRect.X * 0.19f * xPercent) / 2.0f,
                centerPoint.y - (40.0f * yPercent) - (currentRankRect.Y * 0.19f * yPercent) / 2.0f
            ));
            ImGui::Image(currentRankTex, ImVec2(
                currentRankRect.X * 0.19f * xPercent,
                currentRankRect.Y * 0.19f * yPercent
            ));
        }
    }

    ImGui::End();

    if (!isWindowOpen_) {
        cvarManager->executeCommand("togglemenu " + GetMenuName());
    }
}

// ============================================================================
// MENU INTERFACE IMPLEMENTATION
// ============================================================================

std::string LadderRank::GetMenuName() {
    return "LadderRank";
}

std::string LadderRank::GetMenuTitle() {
    return "LadderRank";
}

void LadderRank::SetImGuiContext(uintptr_t ctx) {
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool LadderRank::ShouldBlockInput() {
    return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

bool LadderRank::IsActiveOverlay() {
    return false;
}

void LadderRank::OnOpen() {
    isOpen = true;
}

void LadderRank::OnClose() {
    isOpen = false;
}
#include "hge_internal.h"
#include "guichan/hge/hgeinput.hpp"
#include "../../WapMap/windows/coordinateNavigation.h"
#include "../../WapMap/windows/planeGeometry.h"
#include "../../WapMap/windows/tilePropertyGeometry.h"
#include "../../WapMap/windows/mediaPlayback.h"
#include "../../WapMap/windows/objectToolGeometry.h"
#include "../../WapMap/windows/objectSearch.h"
#include "../../WapMap/windows/mapShotValidation.h"
#include "../../WapMap/windows/mapShotImageWriter.h"
#include "../../WapMap/objEdit/pickRectGeometry.h"
#include "../../WapMap/objEdit/timedSeries.h"
#include "../../WapMap/objEdit/enemyGeometry.h"
#include "../../WapMap/objEdit/ambientTiming.h"
#include "../../WapMap/objEdit/textLayout.h"
#include "../../WapMap/objEdit/cannonGeometry.h"
#include "../../WapMap/objEdit/candySelection.h"
#include "../../WapMap/objEdit/elevatorPathGeometry.h"
#include "../../WapMap/objEdit/elevatorGeometry.h"
#include "../../WapMap/windows/imageImportData.h"
#include "../../WapMap/windows/imageImportFiles.h"
#include "../../shared/cPID.h"
#include "../../WapMap/windows/mapStatistics.h"
#include "../../WapMap/windows/chromeGeometry.h"
#include <fstream>
#include <SFML/Graphics/Image.hpp>
#include <iostream>
#include <stdexcept>

// Isolate graphics and the real guichan adapter from the application loop.
HGE::HGE() : impl_(std::make_unique<Impl>()) {}
HGE::~HGE() = default;
bool HGE::System_GetState(hgeBoolState) { return false; }
float HGE::Timer_GetTime() { return 1.f; }
bool HGE::Input_GetEvent(hgeInputEvent*) { return false; }
HGE* hge;

class InputProbe : public gcn::HGEInput {
public:
    void feed(int type, int key = 0, int wheel = 0, int x = 10, int y = 10) {
        hgeInputEvent event{type, key, 0, 0, wheel, static_cast<float>(x), static_cast<float>(y)};
        pollMouseInput(&event);
    }
};

static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

static void fill(HGE& renderer, DWORD color) {
    hgeQuad quad{};
    quad.blend = BLEND_DEFAULT;
    quad.v[0].x = quad.v[3].x = 0;
    quad.v[1].x = quad.v[2].x = 32;
    quad.v[0].y = quad.v[1].y = 0;
    quad.v[2].y = quad.v[3].y = 32;
    for (auto& vertex : quad.v) vertex.col = color;
    renderer.Gfx_RenderQuad(&quad);
}

int main() {
    try {
        HGE renderer;
        hge = &renderer;
        check(std::abs(ChromeGeometry::Zoom(-5)-.1f)<.0001f && ChromeGeometry::Zoom(0)==1 && ChromeGeometry::Zoom(5)==2,
              "Zoom slider preserves its minimum, neutral and maximum scales");
        check(std::abs(ChromeGeometry::Zoom(1)-1.2f)<.0001f && ChromeGeometry::Zoom(.99f)<ChromeGeometry::Zoom(1.01f),
              "Zoom slider is continuous through positive one instead of resetting to neutral");
        MapStatistics::Counts tileStats{};
        const std::array<unsigned char,3> mixedAttributes{1,1,4};
        MapStatistics::AddTile(tileStats,true,false,{});
        MapStatistics::AddTile(tileStats,false,true,{});
        MapStatistics::AddTile(tileStats,false,false,mixedAttributes);
        MapStatistics::AddTile(tileStats,false,false,{},false);
        check(tileStats==MapStatistics::Counts{1,2,0,0,1,1}, "Statistics distinguish empty, filled, mixed and missing tile attributes without duplicate membership");
        check(MapStatistics::Treasure("GAME_TREASURE_COINS")==0 && MapStatistics::Treasure("game_treasure_jeweledskull")==9 && !MapStatistics::Treasure("LEVEL_ELEVATORS"), "Statistics classify treasure image sets case-insensitively");
        std::array<std::int64_t,10> treasureStats{}; treasureStats[9]=1000000;
        check(MapStatistics::Score(treasureStats)==25000000000LL, "Statistics scores do not overflow 32-bit arithmetic");
        check(ImageImportData::FilenameID("folder99/frame0012.pid") == 12 && ImageImportData::FilenameID("C:\\42\\zero0.png") == 0,
              "Image import derives ID from basename without including directory digits");
        check(!ImageImportData::ID("32768",32767) && !ImageImportData::ID("1x") && !ImageImportData::ID("-1"), "Image import rejects truncated, negative and overflowing IDs");
        check(ImageImportData::Name("props/trees",true) && !ImageImportData::Name("../trees",true) && !ImageImportData::Name("C:/trees",true) && !ImageImportData::Name("CON",false), "Image import destination remains a valid relative folder");
        ImageImportData::Image importImage;
        importImage.width=257; importImage.height=2; importImage.user={17,29,0,0};
        importImage.pixels.resize(514);
        for(size_t i=0;i<importImage.pixels.size();++i) importImage.pixels[i]=i%3?static_cast<unsigned char>(i):0;
        const auto encodedImport=ImageImportData::EncodePID(importImage);
        auto decodedImport=ImageImportData::PID(encodedImport);
        check(decodedImport && decodedImport->pixels==importImage.pixels && decodedImport->user==importImage.user, "Imported PID preserves palette indices, long runs and image offsets");
        PID::Image legacyImport(encodedImport.data(),static_cast<int>(encodedImport.size()));
        check(legacyImport.GetWidth()==257 && legacyImport.GetHeight()==2 && legacyImport.GetOffsetX()==17 && legacyImport.GetOffsetY()==29,
              "New import output is readable by the application's existing PID codec");
        for(int y=0;y<2;++y) for(int x=0;x<257;++x)
            check(legacyImport.GetColorIdAt(x,y)==importImage.pixels[size_t(y)*257+x], "Existing PID codec reads every imported palette index");
        auto truncatedImport=encodedImport; truncatedImport.pop_back();
        check(!ImageImportData::PID(truncatedImport), "Truncated PID input is rejected without reading outside its bytes");
        ImageImportData::Bytes pcx(128,0);
        pcx[0]=10; pcx[2]=1; pcx[3]=8; pcx[8]=1; pcx[10]=1; pcx[65]=1; pcx[66]=4;
        pcx.insert(pcx.end(),{1,2,9,9,3,4,9,9}); pcx.push_back(12); pcx.resize(pcx.size()+768);
        const auto decodedPcx=ImageImportData::PCX(pcx);
        check(decodedPcx && decodedPcx->pixels==ImageImportData::Bytes({1,2,3,4}), "PCX row padding is excluded from image pixels");
        pcx[128]=255;
        check(!ImageImportData::PCX(pcx), "PCX run exceeding decoded image bounds is rejected");
        ImageImportData::Palette importPalette{}; importPalette.fill(0xFF000000); importPalette[7]=0xFFFF0000;
        check(ImageImportData::Closest(0xFFFF0000,importPalette,0)==7 && ImageImportData::Closest(0x00FF0000,importPalette,0)==0, "Palette conversion preserves exact colors and alpha transparency");
        const auto importDirectory=std::filesystem::temp_directory_path()/("wapmap-import-test-"+std::to_string(GetCurrentProcessId()));
        std::string importError;
        check(ImageImportFiles::Save(importDirectory,{{17,encodedImport}},importError), "Image import writes its staged PID batch");
        check(!ImageImportFiles::Save(importDirectory,{{17,{1,2,3}}},importError) && std::filesystem::file_size(importDirectory/"17.pid")==encodedImport.size(), "Import never overwrites an existing custom file");
        check(!ImageImportFiles::Save(importDirectory,{{18,encodedImport},{18,encodedImport}},importError) && !std::filesystem::exists(importDirectory/"18.pid"), "A failed import batch rolls back files it already committed");
        std::filesystem::remove(importDirectory/"17.pid"); std::filesystem::remove(importDirectory);
        check(ElevatorGeometry::Range(100, 75, true) == std::pair{25, 100}, "Elevator leftward bounds");
        check(ElevatorGeometry::Range(100, 75, false) == std::pair{100, 175}, "Elevator rightward bounds");
        check(!ElevatorGeometry::Range(INT_MAX, 1, false) && !ElevatorGeometry::Range(INT_MIN, 1, true),
              "Elevator bounds reject coordinate overflow");
        check(ElevatorGeometry::Scale(100, 2, true) == 50 && ElevatorGeometry::Scale(50, 2, false) == 100,
              "Elevator locked X/Y fields preserve the ratio in both directions");
        check(ElevatorGeometry::Scale(0, 2, true) == 0, "Elevator locked fields accept zero");
        check(!ElevatorGeometry::Scale(100, 0, true) && !ElevatorGeometry::Scale(INT_MAX, 2, false),
              "Elevator locked fields reject zero ratio and scaling overflow");
        using ElevatorPathGeometry::Steps;
        auto closure = ElevatorPathGeometry::Close(Steps{{6, 100}, {2, 50}}, 500, 500);
        auto endpoint = closure ? ElevatorPathGeometry::End(closure->steps, 500, 500) : std::nullopt;
        check(endpoint && endpoint->x == 500 && endpoint->y == 500 && !closure->removed,
              "Elevator path closes unequal diagonal displacement in two return steps");
        closure = ElevatorPathGeometry::Close(Steps(8, {6, 100}), 100, 100);
        endpoint = closure ? ElevatorPathGeometry::End(closure->steps, 100, 100) : std::nullopt;
        check(endpoint && endpoint->x == 100 && endpoint->y == 100 && closure->removed && closure->steps.size() <= 8,
              "Elevator closure respects eight-step storage and reports removed steps");
        check(!ElevatorPathGeometry::End(Steps{{6, 100}, {4, 100}}, INT_MAX - 50, 100),
              "Elevator rejects overflowing intermediate points even with a valid final point");
        check(!ElevatorPathGeometry::End(Steps{{4, INT_MIN}}, 0, 0), "Malformed negative path distance is rejected");
        check(!ElevatorPathGeometry::Drag(0, 0), "Stationary elevator path drag preserves the step");
        check(ElevatorPathGeometry::Drag(100, 10) == std::pair{6, 100}, "Elevator drag snaps horizontal");
        check(ElevatorPathGeometry::Drag(-60, -80) == std::pair{7, 80}, "Elevator drag snaps diagonal");
        check(!ElevatorPathGeometry::Drag(static_cast<std::int64_t>(INT_MAX) - INT_MIN, 0), "Elevator drag rejects unrepresentable distance");
        auto candy = CandySelection::Image(3, 17);
        check(candy && !candy->animated && candy->frame == 17, "Candy explicit frame preserves its asset ID");
        candy = CandySelection::Image(3, std::nullopt);
        check(candy && candy->animated && candy->frame == -1, "Animated candy clears the previous explicit frame");
        candy = CandySelection::Image(1, std::nullopt);
        check(candy && !candy->animated && candy->frame == -1, "Single-frame candy uses automatic frame without animation");
        check(!CandySelection::Image(0, std::nullopt), "Empty candy image set cannot be picked");
        for (const bool right : {false, true}) for (const int angle : {0, 45, 90, 135, 180, 225, 270, 315}) {
            const auto polar = CannonGeometry::Polar(1000, angle, right);
            const auto measured = polar ? CannonGeometry::Components(polar->x, polar->y, right) : std::nullopt;
            check(measured && measured->angle == angle, "Cannon angle/component conversion round-trips for both facings");
            check(polar->worldX == (right ? polar->x : -polar->x), "Cannon facing mirrors X without reversing Y");
        }
        check(CannonGeometry::Normalize(-450) == 270 && CannonGeometry::Normalize(720) == 0, "Cannon angle normalization");
        check(!CannonGeometry::Components(-2147483647 - 1, 0, false), "Cannon rejects unrepresentable mirrored component");
        check(!CannonGeometry::Components(2147483647, 2147483647, true), "Cannon rejects unrepresentable magnitude");
        const auto cannonDrag = CannonGeometry::Drag(180, 160, 100, 100, false, true);
        check(cannonDrag && cannonDrag->worldX == 60 && cannonDrag->x == -60 && cannonDrag->y == 60, "Shift drag snaps diagonal before converting to local cannon coordinates");
        const auto cannonHorizontal = CannonGeometry::Drag(200, 160, 100, 100, true, true);
        check(cannonHorizontal && cannonHorizontal->x == 100 && cannonHorizontal->y == 0, "Shift drag retains horizontal snap threshold");
        check(!CannonGeometry::Drag(2147483647, 0, -2147483647, 0, true, false), "Cannon drag rejects coordinate-difference overflow");
        const auto glyphWidth = [](int) { return 6; };
        auto mapText = MapTextLayout::Build(" \n~A", 100, 200, 0, glyphWidth);
        check(mapText && mapText->glyphs.size() == 1 && mapText->glyphs[0].frame == 1 && mapText->glyphs[0].x == 110 && mapText->glyphs[0].y == 220, "Leading whitespace/unsupported bytes do not add a phantom text object");
        mapText = MapTextLayout::Build("AA\nA", 0, 0, 1, glyphWidth);
        check(mapText && mapText->glyphs[0].x == 10 && mapText->glyphs[2].x == 13, "Map text centers each line within longest line");
        mapText = MapTextLayout::Build("AA\nA", 0, 0, 2, glyphWidth);
        check(mapText && mapText->glyphs[2].x == 16, "Map text right alignment");
        mapText = MapTextLayout::Build("..", 0, 0, 0, glyphWidth);
        check(mapText && mapText->glyphs[0].y == mapText->glyphs[1].y, "Initial period uses same baseline as subsequent periods");
        check(!MapTextLayout::Build(" \n~", 0, 0, 0, glyphWidth), "Unsupported-only text cannot create a placeholder object");
        check(!MapTextLayout::Build("AA", 2147483640, 0, 0, glyphWidth), "Text layout rejects coordinate overflow before mutating objects");
        check(MapTextLayout::Frame('a') == 1 && MapTextLayout::Frame('9') == 36 && MapTextLayout::Frame('*') == 54, "Game-font frame mapping remains compatible");
        check(AmbientTiming::ValidInterval(0, 100), "Ambient interval accepts immediate minimum");
        check(!AmbientTiming::ValidInterval(100, 100), "Ambient interval rejects equal endpoints");
        check(!AmbientTiming::ValidInterval(101, 100) && !AmbientTiming::ValidInterval(-1, 100), "Ambient interval rejects reversed/negative bounds");
        check(EnemyGeometry::InvalidDestination(100, 129, 640, 128) == std::array<bool, 2>{false, true}, "Enemy Y destination uses height on a wide map");
        check(EnemyGeometry::InvalidDestination(100, 500, 128, 640) == std::array<bool, 2>{false, false}, "Tall map accepts Y beyond its width");
        check(EnemyGeometry::InvalidDestination(0, 640, 128, 640) == std::array<bool, 2>{true, true}, "Enemy destination excludes zero and far edge");
        check(TimedSeries::Delay("SawBlade2", 0, "SawBlade") == 750, "Saw preset delay survives Save/Add next reconstruction");
        check(TimedSeries::Delay("SawBlade3", 0, "SawBlade") + 250 == 1750, "Saw series increments decoded preset delay");
        check(TimedSeries::Delay("FloorSaw4", 0, "SawBlade") == 2250, "Legacy saw preset alias remains supported");
        check(TimedSeries::Delay("FloorSpike3", 0, "FloorSpike") == 1500, "Spike preset delay");
        check(TimedSeries::Delay("TogglePeg4", 0, "TogglePeg") == 2250, "Peg preset delay");
        check(TimedSeries::Delay("TogglePeg", 900, "TogglePeg") == 900, "Explicit series delay remains unchanged");
        auto dragBounds = PickRectGeometry::FromDrag(100, 100, 30, 20);
        check(dragBounds.x1 == 30 && dragBounds.y1 == 20 && dragBounds.x2 == 100 && dragBounds.y2 == 100, "Rectangle supports reverse drag");
        dragBounds = PickRectGeometry::FromDrag(100, 100, 140, 170);
        check(dragBounds.x1 == 100 && dragBounds.y1 == 100 && dragBounds.x2 == 140 && dragBounds.y2 == 170, "Crossing drag origin resets both near edges");
        check(PickRectGeometry::Invalid({"0", "0", "0", "0"}, true, false) == std::array<bool, 4>{false, false, false, false}, "Optional empty rectangle");
        check(PickRectGeometry::Invalid({"0", "20", "0", "50"}, false, true) == std::array<bool, 4>{false, false, false, false}, "One empty axis remains allowed");
        check(PickRectGeometry::Invalid({"80", "20", "30", "50"}, false, false) == std::array<bool, 4>{true, false, true, false}, "Reversed axis marks both endpoints");
        check(PickRectGeometry::Invalid({"0", "20", "30", "50"}, false, true)[0], "Half-empty axis is invalid");
        check(PickRectGeometry::Invalid({"2147483648", "20", "30", "50"}, false, false)[0], "Overflowing coordinate is invalid");
        check(PickRectGeometry::Invalid({"-1", "20", "30", "50"}, false, false)[0], "Negative coordinate is invalid");
        int mediaFrame = 0; double mediaElapsed = 0; bool mediaPlaying = true;
        const int durations[] = {10, 20, 30};
        MediaPlayback::Advance(3, mediaFrame, mediaElapsed, mediaPlaying, false, 35, [&](int i) { return durations[i]; });
        check(mediaFrame == 2 && mediaElapsed == 5 && mediaPlaying, "Animation respects variable frame durations");
        MediaPlayback::Advance(3, mediaFrame, mediaElapsed, mediaPlaying, false, 100, [&](int i) { return durations[i]; });
        check(mediaFrame == 2 && !mediaPlaying, "Non-looping playback holds the final frame");
        mediaFrame = 0; mediaElapsed = 0; mediaPlaying = true;
        MediaPlayback::Advance(2, mediaFrame, mediaElapsed, mediaPlaying, true, 10000001, [](int) { return 0; });
        check(mediaFrame == 1 && mediaElapsed == 0 && mediaPlaying, "Zero-duration frames and large time steps do not hang");
        MediaPlayback::Advance(0, mediaFrame, mediaElapsed, mediaPlaying, true, 10, [](int) { return 1; });
        check(mediaFrame == -1 && !mediaPlaying, "Empty animations stop safely");
        check(MediaPlayback::ImageSet("STAND", "", false, [](const std::string&) { return false; }) == "STAND", "Missing image-set separator is safe");
        check(MediaPlayback::ImageSet("unused", "LEVEL13_IMAGES_SOLDIER", false, [](const std::string& value) { return value == "LEVEL_SOLDIER"; }) == "LEVEL_SOLDIER", "Resolve declared level-relative image sets");
        check(TilePropertyGeometry::ValidSize(64, 64, true), "Normal mask dimensions");
        check(!TilePropertyGeometry::ValidSize(-1, 64, true), "Reject negative mask allocation");
        check(!TilePropertyGeometry::ValidSize(9999, 9999, true), "Reject excessive mask allocation");
        const int source[] = {1, 2, 3, 4, 5, 6};
        std::vector<int> grown(12, 0);
        TilePropertyGeometry::PreserveMask(source, 3, 2, 4, 3,
            [&](int x, int y, int value) { grown[y * 4 + x] = value; });
        check(grown == std::vector<int>({1, 2, 3, 0, 4, 5, 6, 0, 0, 0, 0, 0}), "Growing a mask preserves rows and clears new cells");
        std::vector<int> shrunk(4, 0);
        TilePropertyGeometry::PreserveMask(source, 3, 2, 2, 2,
            [&](int x, int y, int value) { shrunk[y * 2 + x] = value; });
        check(shrunk == std::vector<int>({1, 2, 4, 5}), "Shrinking a mask crops with the original stride");
        check(PlaneArithmeticFits(469, 120, 64, 64), "Existing large level dimensions fit");
        check(!PlaneArithmeticFits(99999, 99999, 64, 64), "Reject overflowing tile count");
        check(!PlaneArithmeticFits(99999, 1, 99999, 64), "Reject overflowing pixel width");
        check(!PlaneArithmeticFits(1, 99999, 64, 99999), "Reject overflowing pixel height");
        check(PlaneArithmeticFits(0, 100, 64, 64), "Delegate empty dimensions to legacy validation");
        check(ObjectToolGeometry::CanOffset(100, -25, 4), "Duplicate offsets may move through zero");
        check(ObjectToolGeometry::CanOffset(2147483640, 7, 1), "Series delay may reach signed maximum");
        check(!ObjectToolGeometry::CanOffset(2147483640, 8, 1), "Series delay increment must not overflow");
        check(ObjectToolGeometry::CanOffset(0, 1, 2147483647), "Duplicate offset accepts signed upper boundary");
        check(!ObjectToolGeometry::CanOffset(1, 1, 2147483647), "Reject duplicate final-position overflow");
        check(!ObjectToolGeometry::CanOffset(-2, -2147483647, 1), "Reject duplicate lower position overflow");
        check(!ObjectToolGeometry::CanOffset(-2147483647, 2147483647, 2), "Reject intermediate offset overflow even if final sum fits");
        check(ObjectSearch::Match("Checkpoint", "POINT", false), "Object search case-insensitive substring");
        check(!ObjectSearch::Match("Checkpoint", "POINT", true), "Object search preserves case-sensitive option");
        check(!ObjectSearch::Match("Checkpoint", "", false), "Empty object query does not match everything");
        check(ObjectSearch::MatchId(42, "42"), "Search matches exact object ID");
        check(!ObjectSearch::MatchId(42, "42x"), "Object ID search rejects trailing text");
        check(!ObjectSearch::MatchId(42, "999999999999999999"), "Object ID search rejects overflow without throwing");
        check(!MapShotValidation::Dimensions(1, 1, 0.1), "Reject zero-pixel mapshot output after scaling");
        check(!MapShotValidation::Dimensions(100, 100, 0), "Reject zero mapshot scale");
        check(MapShotValidation::Dimensions(400, 200, 25) == std::pair{100, 50}, "Mapshot dimensions follow scale");
        const auto probePath = std::filesystem::temp_directory_path() / ("wapmap-mapshot-probe-" + std::to_string(GetCurrentProcessId()) + ".png");
        { std::ofstream probe(probePath); probe << "keep-existing-image"; }
        check(MapShotValidation::CanSave(probePath.string()), "Existing writable mapshot destination is valid");
        std::string preserved;
        { std::ifstream probe(probePath); probe >> preserved; }
        check(preserved == "keep-existing-image", "Mapshot path validation must not delete or truncate existing files");
        std::filesystem::remove(probePath);
        check(!MapShotValidation::CanSave(std::filesystem::temp_directory_path().string()), "Mapshot destination cannot be a directory");
        auto coordinate = CoordinateNavigation::Resolve("2", "3", false, 64, 32);
        check(coordinate && coordinate->x == 160 && coordinate->y == 112, "Navigate to tile center");
        coordinate = CoordinateNavigation::Resolve("", "", true, 64, 32);
        check(coordinate && coordinate->x == 0 && coordinate->y == 0, "Empty coordinate is zero");
        coordinate = CoordinateNavigation::Resolve("-100", "2147483647", true, 64, 32);
        check(coordinate && coordinate->x == -100 && coordinate->y == 2147483647, "Signed world coordinates");
        check(!CoordinateNavigation::Resolve("2147483647", "0", false, 64, 32), "Reject tile multiplication overflow");
        check(!CoordinateNavigation::Resolve("2147483648", "0", true, 64, 32), "Reject out of range input");
        check(!CoordinateNavigation::Resolve("12x", "0", true, 64, 32), "Reject partially numeric text");
        check(!CoordinateNavigation::Resolve("-", "0", true, 64, 32), "Reject unfinished negative number");
        check(!CoordinateNavigation::Resolve("0", "0", false, 0, 32), "Reject invalid tile dimensions");
        InputProbe input;
        input.feed(INPUT_MBUTTONDOWN, HGEK_LBUTTON);
        input.feed(INPUT_MBUTTONDOWN, HGEK_RBUTTON);
        input.feed(INPUT_MBUTTONUP, HGEK_LBUTTON);
        input.feed(INPUT_KEYUP, HGEK_RBUTTON); // Focus loss releases mouse buttons too.
        check(input.dequeueMouseInput().getType() == gcn::MouseInput::MOVED, "Initial movement");
        const int expected[] = {gcn::MouseInput::PRESSED, gcn::MouseInput::PRESSED,
                                gcn::MouseInput::RELEASED, gcn::MouseInput::RELEASED};
        for (int type : expected) check(input.dequeueMouseInput().getType() == type, "Ordered button transitions");
        check(input.isMouseQueueEmpty(), "No duplicate button transitions");
        input.feed(INPUT_MOUSEWHEEL, 0, 2);
        input.feed(INPUT_KEYDOWN, HGEK_A);
        for (int i = 0; i < 2; ++i)
            check(input.dequeueMouseInput().getType() == gcn::MouseInput::WHEEL_MOVED_UP, "Wheel delta");
        check(input.isMouseQueueEmpty(), "Keyboard events must not repeat the wheel");
        input.feed(INPUT_MOUSEMOVE, 0, 0, -1, -1);
        auto leave = input.dequeueMouseInput();
        check(leave.getX() == -1 && leave.getY() == -1, "Mouse leave reaches guichan");

        const auto chunkPlan = MapShotValidation::Plan(3, 3, 2);
        check(chunkPlan && chunkPlan->columns == 2 && chunkPlan->rows == 2, "Mapshot chunk layout covers partial edges");
        check(chunkPlan->Bounds(3) == std::array{2, 2, 1, 1}, "Final mapshot chunk crops both dimensions");
        check(!MapShotValidation::Plan(2147483647, 2147483647, 1), "Reject mapshot chunk-count overflow");
        auto chunkTarget = renderer.Target_Create(2, 2, false);
        sf::Image stitched({3, 3}, sf::Color::Transparent);
        const DWORD chunkColors[] = {0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffffff};
        for (int i = 0; i < 4; ++i) {
            check(renderer.Gfx_BeginScene(chunkTarget), "Mapshot chunk target remains usable across passes");
            renderer.Gfx_Clear(chunkColors[i]);
            renderer.Gfx_EndScene();
            const auto* chunkPixels = renderer.Surface_Lock(renderer.Target_GetSurface(chunkTarget));
            check(chunkPixels, "Read mapshot chunk pixels");
            const auto bounds = chunkPlan->Bounds(i);
            for (int y = 0; y < bounds[3]; ++y) for (int x = 0; x < bounds[2]; ++x) {
                const auto color = chunkPixels[y * 2 + x];
                stitched.setPixel({static_cast<unsigned>(bounds[0] + x), static_cast<unsigned>(bounds[1] + y)}, sf::Color(GETR(color), GETG(color), GETB(color)));
            }
        }
        renderer.Target_Free(chunkTarget);
        const auto imagePath = std::filesystem::temp_directory_path() / ("wapmap-mapshot-roundtrip-" + std::to_string(GetCurrentProcessId()) + ".png");
        check(stitched.saveToFile(imagePath), "Save mapshot to full destination path");
        const sf::Image loaded(imagePath);
        check(loaded.getSize() == sf::Vector2u{3, 3}, "Mapshot roundtrip dimensions");
        check(loaded.getPixel({0, 0}) == sf::Color::Red && loaded.getPixel({2, 0}) == sf::Color::Green &&
            loaded.getPixel({0, 2}) == sf::Color::Blue && loaded.getPixel({2, 2}) == sf::Color::White, "Mapshot chunk edges survive PNG roundtrip");
        std::filesystem::remove(imagePath);
        const auto codecDir = std::filesystem::temp_directory_path() / (L"wapmap-codec-" + std::to_wstring(GetCurrentProcessId()));
        std::filesystem::create_directory(codecDir);
        sf::Image pattern(sf::Vector2u{64, 64});
        for (unsigned y = 0; y < 64; ++y) for (unsigned x = 0; x < 64; ++x)
            pattern.setPixel({x, y}, sf::Color(static_cast<unsigned char>(x * 4), static_cast<unsigned char>(y * 4), static_cast<unsigned char>((x * 31 + y * 19) % 256), static_cast<unsigned char>(x + y)));
        const auto rawPng = codecDir / L"bez kompresji.png", compressedPng = codecDir / L"obraz-\u0142.png";
        check(MapShotImageWriter::Save(pattern, rawPng, 0), "Encode uncompressed PNG");
        check(MapShotImageWriter::Save(pattern, compressedPng, 9), "Encode compressed PNG at Unicode path");
        check(std::filesystem::file_size(compressedPng) < std::filesystem::file_size(rawPng), "PNG compression option changes output size");
        sf::Image pngRoundtrip(compressedPng);
        check(pngRoundtrip.getPixel({31, 37}) == pattern.getPixel({31, 37}), "PNG encoding preserves RGBA pixels");
        const auto lowJpeg = codecDir / "low.jpg", highJpeg = codecDir / "high.jpeg";
        check(MapShotImageWriter::Save(pattern, lowJpeg, 10) && MapShotImageWriter::Save(pattern, highJpeg, 95), "Encode both JPEG quality settings");
        check(std::filesystem::file_size(lowJpeg) < std::filesystem::file_size(highJpeg), "JPEG quality option changes output size");
        check(sf::Image(highJpeg).getSize() == pattern.getSize(), "JPEG decodes with original dimensions");
        const HANDLE locked = CreateFileW(compressedPng.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        check(locked != INVALID_HANDLE_VALUE, "Lock existing export destination");
        const bool replacedLocked = MapShotImageWriter::Save(sf::Image({64, 64}, sf::Color::Blue), compressedPng, 9);
        CloseHandle(locked);
        check(!replacedLocked && sf::Image(compressedPng).getPixel({31, 37}) == pattern.getPixel({31, 37}), "Failed export commit preserves previous image");
        check(MapShotImageWriter::JpegQuality(0) == 75 && MapShotImageWriter::JpegQuality(7) == 100 && MapShotImageWriter::JpegQuality(100) == 1, "JPEG compression UI maps to valid quality levels");
        for (const auto& path : {rawPng, compressedPng, lowJpeg, highJpeg}) std::filesystem::remove(path);
        check(std::filesystem::is_empty(codecDir), "Export writer cleans up its failed temporary file");
        std::filesystem::remove(codecDir);

        auto target = renderer.Target_Create(32, 32, false);
        check(target != nullptr, "Create render target");
        renderer.Gfx_BeginScene(target);
        check(!renderer.Gfx_BeginScene(target), "Reject nested scenes without changing the active pass");
        int clipX, clipY, clipWidth, clipHeight;
        renderer.Gfx_GetClipping(&clipX, &clipY, &clipWidth, &clipHeight);
        check(clipX == 0 && clipY == 0 && clipWidth == 32 && clipHeight == 32,
              "New scene reports full target bounds for selection border slicing");
        renderer.Gfx_SetClipping(-4, 6, 40, 40);
        renderer.Gfx_GetClipping(&clipX, &clipY, &clipWidth, &clipHeight);
        check(clipX == 0 && clipY == 6 && clipWidth == 32 && clipHeight == 26,
              "Clip query reports bounds intersected with the render target");
        renderer.Gfx_SetClipping();
        renderer.Gfx_GetClipping(&clipX, &clipY, &clipWidth, &clipHeight);
        check(clipX == 0 && clipY == 0 && clipWidth == 32 && clipHeight == 32,
              "Reset clipping reports full target dimensions instead of zero");
        renderer.Gfx_Clear(0xFF000000);
        renderer.Gfx_SetClipping(8, 8, 8, 8);
        fill(renderer, 0xFFFFFFFF);
        renderer.Gfx_EndScene();
        auto pixels = renderer.Surface_Lock(renderer.Target_GetSurface(target));
        check(pixels && pixels[0] == 0xFF000000 && pixels[10 * 32 + 10] == 0xFFFFFFFF,
              "Clipped batch pixels");
        renderer.Gfx_BeginScene(target);
        // Same target, new pass: no clip inherited from the object picker.
        fill(renderer, 0xFFFF0000);
        renderer.Gfx_EndScene();
        pixels = renderer.Surface_Lock(renderer.Target_GetSurface(target));
        check(pixels && pixels[0] == 0xFFFF0000 && pixels[31 * 32 + 31] == 0xFFFF0000,
              "Scene scissor reset");
        renderer.Gfx_BeginScene(target);
        renderer.Gfx_SetTransform(10, 10, 50, 50, 1, 2, 0);
        fill(renderer, 0xFF00FF00);
        renderer.Gfx_EndScene();
        pixels = renderer.Surface_Lock(renderer.Target_GetSurface(target));
        check(pixels && pixels[0] == 0xFF00FF00, "HGE identity transform sentinel");
        renderer.Gfx_BeginScene(target);
        renderer.Gfx_SetTransform(0, 0, 64, 64, 0, 1, 1);
        renderer.Gfx_EndScene();
        renderer.Gfx_BeginScene(target);
        fill(renderer, 0xFF0000FF);
        renderer.Gfx_EndScene();
        pixels = renderer.Surface_Lock(renderer.Target_GetSurface(target));
        check(pixels && pixels[0] == 0xFF0000FF, "Scene transform reset");
        // Cancel is the last visible draw and leaves an actual GL scissor active.
        // ImGui-SFML saves GL states, resets SFML's cache, then restores the GL
        // scissor without restoring that cache. Exercise this exact boundary.
        renderer.Gfx_BeginScene(target);
        renderer.Gfx_SetClipping(8, 8, 8, 8);
        fill(renderer, 0xFFFFFFFF);
        renderer.Gfx_SetClipping(); // View reset is deferred until the next draw.
        renderer.Gfx_EndScene();
        target->renderTexture.pushGLStates();
        target->renderTexture.popGLStates();
        renderer.Gfx_BeginScene(target);
        fill(renderer, 0xFFFF00FF);
        renderer.Gfx_EndScene();
        pixels = renderer.Surface_Lock(renderer.Target_GetSurface(target));
        check(pixels && pixels[0] == 0xFFFF00FF && pixels[31 * 32 + 31] == 0xFFFF00FF,
              "Background after ImGui GL state save/restore must not inherit Cancel scissor");
        renderer.Target_Free(target);
        std::cout << "Migration graphics/input regression checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

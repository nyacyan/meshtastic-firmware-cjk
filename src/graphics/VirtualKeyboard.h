#pragma once

#include "configuration.h"
#include <OLEDDisplay.h>
#include <functional>
#include <string>
#include <vector>

// Which Chinese IME backend drives the on-screen candidate machinery in this
// class: Bopomofo/zhuyin for the joystick builds, or the upstream pinyin backend.
// The two are mutually exclusive. Builds that define neither compile a plain Latin
// keyboard; the Cardputer is such a build - its BopomofoInputModule subclass drives
// its own composer and only needs the base keyboard plus a few virtual hooks.
#if defined(CJK_IME_PINYIN) && defined(CJK_IME_ZHUYIN)
#error "CJK_IME_PINYIN and CJK_IME_ZHUYIN are mutually exclusive"
#endif

#if defined(CJK_IME_PINYIN) || defined(CJK_IME_ZHUYIN)
#define VK_HAS_CJK_IME 1
#endif

#if defined(CJK_IME_ZHUYIN)
#include "bpmf_engine.h"
#endif

// Boards driven by a joystick alone: every key has to be reached with the four
// directions, and there is no Tab key to jump between them. The user button takes
// over that job. Boards with a real keypad (T-Deck's QWERTY, the GAT562 T9) keep the
// plain one-column nudge instead.
// GAT562 gets this by default; every other board opts in with -DVK_JOYSTICK_ONLY=1.
// Deliberately not probed from HAS_TRACKBALL: several boards carry a trackball
// without wanting this navigation model, and a probe would change their behaviour
// without anyone asking for it.
#if defined(GAT562) && !defined(GAT562_T9_KEYBOARD)
#define VK_JOYSTICK_ONLY 1
#endif

namespace graphics
{

enum VirtualKeyType { VK_CHAR, VK_BACKSPACE, VK_ENTER, VK_SHIFT, VK_ESC, VK_SPACE };

struct VirtualKey {
    char character;
    VirtualKeyType type;
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
};

class VirtualKeyboard
{
  public:
    VirtualKeyboard();
    virtual ~VirtualKeyboard();

    virtual void draw(OLEDDisplay *display, int16_t offsetX, int16_t offsetY);
    void setInputText(const std::string &text);
    std::string getInputText() const;
    std::string getHeader() const { return headerText; }
    void setHeader(const std::string &header);
    void setCallback(std::function<void(const std::string &)> callback);
    std::function<void(const std::string &)> getCallback() const { return onTextEntered; }

    // Navigation methods for encoder input
    void moveCursorUp();
    void moveCursorDown();
    virtual void moveCursorLeft();
    virtual void moveCursorRight();
#if defined(VK_JOYSTICK_ONLY)
    void moveCursorNext();
#endif
    virtual void handlePress();
    virtual void handleLongPress();
    void handleBackspace();
    void handleCharacter(char c);
#if defined(GAT562_T9_KEYBOARD)
    void handleT9Character(char c);
#endif

    // Physical key character input (override in subclasses for physical keyboards).
    virtual bool handleKeyChar(char) { return false; }

    // Timeout management
    void resetTimeout();
    bool isTimedOut() const;

    // Chinese IME
    virtual void toggleIME();

  protected:
    void submitText();
    void cancelInput(); // Trigger callback with empty string (cancel/exit path)
    void deleteCharacter();

  private:
    static const uint8_t KEYBOARD_ROWS = 4;
    static const uint8_t KEYBOARD_COLS = 11;
    static const uint8_t KEY_WIDTH = 9;
    static const uint8_t KEY_HEIGHT = 9;        // Compressed to fit 4 rows on 64px displays
    static const uint8_t KEYBOARD_START_Y = 26; // Start just below input box bottom

    VirtualKey keyboard[KEYBOARD_ROWS][KEYBOARD_COLS];

    std::string inputText;
    std::string headerText;
    std::function<void(const std::string &)> onTextEntered;

    uint8_t cursorRow;
    uint8_t cursorCol;
#if defined(GAT562_T9_KEYBOARD)
    uint8_t candidateCursor;
    uint8_t lastT9Group = 0;
    uint32_t lastT9Millis = 0;
#endif

    // processedWords / inputTextLayout track the UTF-8 segmentation of the input
    // buffer and are used by the plain Latin path too, so they stay unconditional.
    uint8_t processedWords = 0;
    std::vector<uint8_t> inputTextLayout = {};
#if defined(VK_HAS_CJK_IME)
    // Zhuyin builds start in Chinese and can be toggled to Latin; the pinyin path
    // keeps its existing Latin default.
#if defined(CJK_IME_ZHUYIN)
    enum _IMEStatus { ACTIVE, INACTIVE } IMEStatus = ACTIVE;
    // The input method's own state - candidate list, which kind of lookup
    // produced it, and the cache that keeps the draw path from repeating the
    // search on every frame. The composition itself stays in inputText, where
    // this keyboard already draws and segments it.
    bpmf::Engine bpmfEngine;
#else
    enum _IMEStatus { ACTIVE, INACTIVE } IMEStatus = INACTIVE;
#endif
#if defined(TINYLORA_ADVANCED_IME)
    int resultsOffset = 0;
    int resultsfulllen = 0;
    std::vector<std::string> displayList = {};
    std::vector<uint8_t> selectionPos = {};
#else
    uint8_t selectableChars = 0;
    int selectListfulllen = 0;
    int selectListOffset = 0;
    std::string selectList = "";
    std::vector<uint8_t> selectListLayout = {};
#endif
#endif

    // Timeout management for auto-exit
    uint32_t lastActivityTime;
    static const uint32_t TIMEOUT_MS = 60000; // 1 minute timeout

    void initializeKeyboard();
    void drawKey(OLEDDisplay *display, const VirtualKey &key, bool selected, int16_t x, int16_t y, uint8_t w, uint8_t h,
                 bool isLastCol);
    void drawInputArea(OLEDDisplay *display, int16_t offsetX, int16_t offsetY, int16_t keyboardStartY);

    // Unified cursor movement helper
    void moveCursorDelta(int dRow, int dCol);
#if defined(GAT562_T9_KEYBOARD)
    bool hasChineseCandidates() const;
    uint8_t chineseCandidateCount() const;
    bool moveCandidateCursor(int delta);
#endif

    char getCharForKey(const VirtualKey &key, bool isLongPress = false);
    void insertCharacter(char c);
    uint8_t getLastUtf8CharLength() const;
    uint8_t getUtf8Length(const char *c, uint8_t pos);
#if defined(VK_HAS_CJK_IME)
#if !defined(TINYLORA_ADVANCED_IME)
    uint8_t getChineseChar(uint8_t c);
#endif
    void selectChineseChar(uint8_t chridx);
    void showNextSelection();
#if defined(CJK_IME_ZHUYIN)
    // Whether the candidates span more than one page. draw() uses it to decide
    // whether to render ">", and handleLongPress() to decide whether a long press
    // on the tenth column pages forward.
    bool hasMultipleCandidatePages() const;
#endif
#if defined(GAT562_T9_KEYBOARD)
    void showPrevSelection();
#endif
#endif
};

} // namespace graphics

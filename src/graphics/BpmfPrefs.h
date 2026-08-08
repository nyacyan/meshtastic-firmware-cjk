#pragma once
#include "configuration.h"

// Both Bopomofo front ends store here: BOPOMOFO_IME is the physical-keyboard one
// (BopomofoInputModule), CJK_IME_ZHUYIN the on-screen grid one (VirtualKeyboard).
// No build defines both. The grid boards have no layout switch, so they read the
// record, change only the input-mode bit, and write the rest back untouched.
#if (defined(BOPOMOFO_IME) || defined(CJK_IME_ZHUYIN)) && HAS_SCREEN

#include "FSCommon.h"
#include "graphics/bpmf_engine.h"

// Which layout the IME is in, and whether it is composing Bopomofo or passing
// Latin through, kept in two bytes of their own rather than in the protobuf
// config: the upstream schema stays untouched and the setting costs nothing on
// the wire.
//
// The record is rewritten only when the IME closes holding a value different
// from the one it read.  Both halves are reachable from a key the user can hold
// down, and on nRF52 the whole filesystem is 28 KB of internal flash (7 pages,
// 128 B blocks) shared with the node database, so a write per keystroke would
// be spending an erase budget that belongs to something else.
//
//   byte 0   format version
//   byte 1   bit 0  composing Bopomofo (1) or passing Latin through (0)
//            bit 1  Eten layout (1) or Daqian (0)
//
// savePrefs() deletes the record before writing it rather than overwriting the
// two bytes in place.  FILE_O_WRITE opens with LFS_O_RDWR | LFS_O_CREAT on
// nRF52, without LFS_O_TRUNC, and a write into a file that already exists does
// not reach the flash there: the first save creates the file and every later one
// silently keeps the original contents.  ESP32 opens the same call with "w" and
// would truncate, so the delete costs it nothing.  This is how the rest of the
// tree writes its small records too, see TransmitHistory.

namespace bpmf
{

struct Prefs {
    Layout layout  = DEFAULT_LAYOUT;
    bool   chinese = true;

    bool operator==(const Prefs &o) const { return layout == o.layout && chinese == o.chinese; }
    bool operator!=(const Prefs &o) const { return !(*this == o); }
};

inline constexpr uint8_t PREFS_VERSION = 1;
inline constexpr const char *PREFS_PATH = "/prefs/ime.dat";

// Missing, unreadable or written by a newer format all mean "use the defaults",
// which is why nothing here reports failure: there is no recovery to attempt
// and no user action that would help.
inline Prefs loadPrefs()
{
    Prefs p;
#ifdef FSCom
    auto f = FSCom.open(PREFS_PATH, FILE_O_READ);
    if (f) {
        uint8_t buf[2] = {0, 0};
        // read() is int on nRF52 and size_t on ESP32; the cast keeps the
        // comparison from picking one of them and warning on the other.
        const int n = (int)f.read(buf, sizeof(buf));
        if (n == (int)sizeof(buf) && buf[0] == PREFS_VERSION) {
            p.chinese = (buf[1] & 0x01) != 0;
            p.layout  = (buf[1] & 0x02) ? LAYOUT_YITIAN : LAYOUT_DAQIAN;
        }
        f.close();
    }
#endif
    return p;
}

inline void savePrefs(const Prefs &p)
{
#ifdef FSCom
    uint8_t buf[2] = {PREFS_VERSION,
                      (uint8_t)((p.chinese ? 0x01 : 0x00) | (p.layout == LAYOUT_YITIAN ? 0x02 : 0x00))};
    // NodeDB creates /prefs before anything else writes there, but the IME can
    // be the first writer on a filesystem that was just formatted.
    FSCom.mkdir("/prefs");
    if (FSCom.exists(PREFS_PATH))
        FSCom.remove(PREFS_PATH);
    auto f = FSCom.open(PREFS_PATH, FILE_O_WRITE);
    if (f) {
        f.write(buf, sizeof(buf));
        f.flush();
        f.close();
    }
#endif
}

} // namespace bpmf

#endif // (BOPOMOFO_IME || CJK_IME_ZHUYIN) && HAS_SCREEN

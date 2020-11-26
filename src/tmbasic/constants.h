#pragma once

#include "../common.h"

namespace tmbasic {

const char kCharBullet = '\x07';
const char kCharStar = '\x0F';

const uint8_t kFgColorBlack = 0x00;
const uint8_t kFgColorBlue = 0x01;
const uint8_t kFgColorGreen = 0x02;
const uint8_t kFgColorCyan = 0x03;
const uint8_t kFgColorRed = 0x04;
const uint8_t kFgColorMagenta = 0x05;
const uint8_t kFgColorBrown = 0x06;
const uint8_t kFgColorWhite = 0x07;
const uint8_t kFgColorGray = 0x08;
const uint8_t kFgColorLightBlue = 0x09;
const uint8_t kFgColorLightGreen = 0x0A;
const uint8_t kFgColorLightCyan = 0x0B;
const uint8_t kFgColorLightRed = 0x0C;
const uint8_t kFgColorPurple = 0x0D;
const uint8_t kFgColorYellow = 0x0E;
const uint8_t kFgColorBrightWhite = 0x0F;
const uint8_t kFgColorMask = 0x0F;

const uint8_t kBgColorBlack = 0x00;
const uint8_t kBgColorBlue = 0x10;
const uint8_t kBgColorGreen = 0x20;
const uint8_t kBgColorCyan = 0x30;
const uint8_t kBgColorRed = 0x40;
const uint8_t kBgColorMagenta = 0x50;
const uint8_t kBgColorYellow = 0x60;
const uint8_t kBgColorWhite = 0x70;
const uint8_t kBgColorMask = 0x70;

const uint8_t kPaletteListViewerActiveInactive = 26;
const uint8_t kPaletteListViewerFocused = 27;
const uint8_t kPaletteListViewerSelected = 28;
const uint8_t kPaletteListViewerDivider = 29;

const uint8_t kPaletteEditorNormal = 6;
const uint8_t kPaletteEditorHighlight = 7;

const char kPaletteBlackDialog[] =
    "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
    "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf";

}  // namespace tmbasic

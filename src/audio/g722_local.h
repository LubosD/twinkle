/*
    Copyright (C) 2019  Frédéric Brière <fbriere@fbriere.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Our own additions to g722.h, kept separate so that the latter can easily
// be updated by copying it as-is from Asterisk

#ifndef _G722_LOCAL_H
#define _G722_LOCAL_H

// A sampling rate of 16 kHz and a bit rate of 64 kbit/s result in a ratio of
// 2 samples per payload byte
#define G722_SAMPLES_PAYLOAD_RATIO (16000 / (64000 / 8))

#endif

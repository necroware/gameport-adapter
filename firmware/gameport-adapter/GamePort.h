// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware 
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

/// Simple GamePort pins to Arduino pins mapper.
template <int I>  
struct GamePort { enum { pin = -1 }; };

template <> struct GamePort<2> { enum { pin = 10 }; };
template <> struct GamePort<3> { enum { pin = A0 }; };
template <> struct GamePort<6> { enum { pin = A7 }; };
template <> struct GamePort<7> { enum { pin = 8 }; };
template <> struct GamePort<10> { enum { pin = 16 }; };
template <> struct GamePort<11> { enum { pin = A1 }; };
template <> struct GamePort<12> { enum { pin = 20 }; };
template <> struct GamePort<13> { enum { pin = A6 }; };
template <> struct GamePort<14> { enum { pin = 7 }; };
template <> struct GamePort<15> { enum { pin = 9 }; };


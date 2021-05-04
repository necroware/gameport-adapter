#pragma once


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


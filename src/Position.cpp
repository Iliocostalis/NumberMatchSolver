#include <Position.h>

template<>
template<>
Position<int>::Position(const Position<float>& other){x = (int)(other.x+0.5f); y = (int)(other.y+0.5f);}
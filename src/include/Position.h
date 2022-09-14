#pragma once

template<class T>
class Position
{
public:
    T x;
    T y;

    Position(){}
    Position(const T& v){this->x = v; this->y = v;}
    Position(const T& x, const T& y){this->x = x; this->y = y;}
    //Position(const Position<T>& other){x = other.x; y = other.y;}
    //void operator=(const Position<T>& other){x = other.x; y = other.y;}
    
    template<class T2>
    Position(const Position<T2>& other);

    template<class T2>
    void operator=(const Position<T2>& other);

    Position<T> operator-(const Position<T>& other)const {return Position<T>(x - other.x, y - other.y);}
    Position<T> operator+(const Position<T>& other)const {return Position<T>(x + other.x, y + other.y);}
    Position<T> operator/(const T& other)const {return Position<T>(x / other, y / other);}
    Position<T> operator*(const T& other)const {return Position<T>(x * other, y * other);}
};

template<class T>
template<class T2>
Position<T>::Position(const Position<T2>& other) {x = other.x; y = other.y;}

template<>
template<class T2>
Position<int>::Position(const Position<T2>& other){x = (int)other.x; y = (int)other.y;}

template<>
template<class T2>
Position<float>::Position(const Position<T2>& other){x = (float)other.x; y = (float)other.y;}

template<class T>
template<class T2>
void Position<T>::operator=(const Position<T2>& other) {x = other.x; y = other.y;}

template<>
template<class T2>
void Position<int>::operator=(const Position<T2>& other) {x = (int)other.x; y = (int)other.y;}

template<>
template<class T2>
void Position<float>::operator=(const Position<T2>& other) {x = (float)other.x; y = (float)other.y;}

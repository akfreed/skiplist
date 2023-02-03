// ===========================================================================
// Copyright (c) 2018 Alexander Freed. ALL RIGHTS RESERVED.
//
// ISO C++11
// 
// Contains a class called Movable that is useful for testing.
// ===========================================================================
#pragma once

#include <ostream>


/** This is a simple class that has a little bit of dynamic memory, is movable,
and has the appropriate overloads to be used as a key/value. It can also be
displayed.
*/
class Movable
{
public:
    // default constructor
    Movable()
    { }

    // argument constructor
    // implicit conversion allowed
    Movable(const int i)
        : m_pi(new int(i))
    { }

    // copy constructor
    Movable(const Movable& other)
        : m_pi(other.m_pi ? new int(*other.m_pi) : nullptr)
    { }

    // move constructor
    Movable(Movable&& other) noexcept
        : m_pi(other.m_pi)
    {
        other.m_pi = nullptr;
    }

    // copy assignment
    Movable& operator=(const Movable& other)
    {
        if (this != &other)
        {
            // deallocate
            delete m_pi;
            // copy
            m_pi = other.m_pi ? new int(*other.m_pi) : nullptr;
        }
        return *this;
    }

    // move assignment
    Movable& operator=(Movable&& other) noexcept
    {
        // deallocate
        delete m_pi;
        // move
        m_pi = other.m_pi;
        other.m_pi = nullptr;
        return *this;
    }

    // destructor
    ~Movable()
    {
        delete m_pi;
    }


    explicit operator bool() const
    {
        return m_pi;
    }


    // getter
    // don't call without first checking for validity with the bool overload!
    int GetVal() const
    {
        if (!m_pi)
        {
            assert(false);
            return ~((unsigned)(-1) >> 1);
        }
        return *m_pi;
    }


    // equality
    bool operator==(const Movable& right) const
    {
        if (!m_pi && !right.m_pi)
            return true;
        if (!m_pi || !right.m_pi)
            return false;
        return *m_pi == *right.m_pi;
    }

    // inequality
    bool operator!=(const Movable& right) const
    {
        return !(*this == right);
    }

    // less than
    // for this class, nullptr is considered less than anything with a value
    bool operator<(const Movable& right) const
    {
        if (!m_pi)
            return true;
        if (!right.m_pi)
            return false;
        return (*m_pi < *right.m_pi);
    }

    // greater than
    // for this class, nullptr is considered less than anything with a value
    bool operator>(const Movable& right) const
    {
        if (!m_pi)  // if nullptr, this can't be greater than anything
            return false;
        if (!right.m_pi)
            return true;
        return (*m_pi > *right.m_pi);
    }

    // less than or equal to
    bool operator<=(const Movable& right) const
    {
        return !(*this > right);
    }

    // display
    friend std::ostream& operator<<(std::ostream& out, const Movable& movable)
    {
        if (!movable.m_pi)
            return out << "NULL";
        return out << *movable.m_pi;
    }


private:
    int* m_pi = nullptr;
};

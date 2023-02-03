// ===========================================================================
// Copyright (c) 2018 Alexander Freed. ALL RIGHTS RESERVED.
//
// ISO C++11
//
// This file defines a custom key/val pair wrapper for std::pair.
// This hierarchy provides better encapsulation for the const pair case,
// while keeping a common interface between iterator and const_iterator.
// It also has move-enabled keys, reasonably fast creation, and the same 
// interface as std::pair.
//
// It uses std::pair for the underlying storage because std::pair already
// implements a std::piecewise_construct_t constructor. To implement this
// constructor, an implementation of std::index_sequence is required. This
// helper is included in the C++14 standard, not C++11. While implementations
// are not terribly difficult, it's safer and more practical to just use the 
// existing implementation.
//
// ===========================================================================
#pragma once

#include <utility>
#include <type_traits>
#include <functional>


namespace fsl {

// ------------------------------------------------------------------
// forward declarations

template <typename KEY, typename VAL>
class SLPair;


// ------------------------------------------------------------------
// Prototypes

template <typename KEY, typename VAL>
void swap(SLPair<KEY, VAL>& left, SLPair<KEY, VAL>& right) noexcept;


// ==================================================================

/** Const Pair
This class is a const pair. A const-qualified instance of this class 
restricts users from modifing m_key and m_val.

NOTE: The copy/move constructors/assignment overloads must be explicit so the 
compiler knows how to set the references. You might not get a compiler error.
*/
template <typename KEY, typename VAL>
class SLPairConst
{
protected:
    // non-public functions
    SLPairConst& operator=(const SLPairConst& other);
    SLPairConst& operator=(SLPairConst&& other) noexcept;
    static void memberSwap(SLPairConst& left, SLPairConst& right) noexcept;

    // non-public data
    std::pair<KEY, VAL> m_keyValPair;

public:
    // standard typedefs
    using first_type  = KEY;
    using second_type = VAL;

    // public functions
    SLPairConst();
    SLPairConst(const SLPairConst& other);
    SLPairConst(SLPairConst&& other) noexcept;
    template <typename FREF_KEY, typename FREF_VAL>
    SLPairConst(FREF_KEY&& key, FREF_VAL&& val);
    SLPairConst(const std::pair<KEY, VAL>& other);
    SLPairConst(std::pair<KEY, VAL>&& other) noexcept;
    template <typename ...KEY_ARGS, typename ...VAL_ARGS>
    SLPairConst(std::piecewise_construct_t, std::tuple<KEY_ARGS...> keyArgs, std::tuple<VAL_ARGS...> valArgs);

    const KEY& GetKey() const;
    const VAL& GetVal() const;

    bool operator==(const SLPairConst& right) const;
    bool operator!=(const SLPairConst& right) const;
    bool operator<(const SLPairConst& right) const;
    bool operator>(const SLPairConst& right) const;
    bool operator<=(const SLPairConst& right) const;
    bool operator>=(const SLPairConst& right) const;

    // public data mimics std::pair
    const KEY& first  = m_keyValPair.first;
    const VAL& second = m_keyValPair.second;
};


// ------------------------------------------------------------------

/** Pair
This is the non-const variety of the pair.
It derives from the const version.

NOTE: The copy/move constructors/assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
*/
template <typename KEY, typename VAL>
class SLPair : public SLPairConst<KEY, VAL>
{
public:
    // standard typedefs
    using first_type  = KEY;
    using second_type = VAL;

    // public functions
    SLPair();
    SLPair(const SLPair& other);
    SLPair(SLPair&& other) noexcept;
    SLPair& operator=(const SLPair& other);
    SLPair& operator=(SLPair&& other) noexcept;
    friend void swap<KEY, VAL>(SLPair& left, SLPair& right) noexcept;
    template <typename FREF_KEY, typename FREF_VAL>
    SLPair(FREF_KEY&& key, FREF_VAL&& val);
    explicit SLPair(const SLPairConst<KEY, VAL>& other);
    SLPair(const std::pair<KEY, VAL>& other);
    SLPair(std::pair<KEY, VAL>&& other) noexcept;
    template <typename ...KEY_ARGS, typename ...VAL_ARGS>
    SLPair(std::piecewise_construct_t, std::tuple<KEY_ARGS...> keyArgs, std::tuple<VAL_ARGS...> valArgs);

    using SLPairConst<KEY, VAL>::GetVal;  // bring const version into this class
    VAL& GetVal();
    void SetVal(VAL val);

    // Public Data 
    VAL& second = this->m_keyValPair.second;  // hides base data
};


// ==================================================================
// make_pair

namespace fsl_utility {

    /** UnwrapRefWrap
    If a type is an std::reference_wrapper, provides a typedef of T&. Otherwise, the typedef is T.
    Base Case
    */
    template <typename T>
    struct UnwrapRefWrap
    {
        using type = T;
    };
    /** UnwrapRefWrap
    If a type is an std::reference_wrapper, provides a typedef of T&. Otherwise, the typedef is T.
    Specialization
    */
    template <typename T>
    struct UnwrapRefWrap<std::reference_wrapper<T>>
    {
        using type = T&;
    };


    /** Helper typedef applies std::decay then _removeRefWrap
    */
    template <typename T>
    using DecayRefWrap_t = typename UnwrapRefWrap<typename std::decay<T>::type>::type;


}  // namespace fsl_utility


/** make_pair
Shorthand function creates an fsl::SLPair, deducing the proper types according to 
the std::make_pair rules--std::decay is applied to the types. If the result is an 
std::reference_wrapper<T>, the type is T&
@param[in] v1 the first element in the pair
@param[in] v2 the second element in the pair
@return an fsl::SLPair comprised of v1 and v2
*/
template <typename T1, typename T2>
SLPair<fsl_utility::DecayRefWrap_t<T1>, fsl_utility::DecayRefWrap_t<T2>> make_pair(T1&& v1, T2&& v2)
{
    return SLPair<fsl_utility::DecayRefWrap_t<T1>, fsl_utility::DecayRefWrap_t<T2>>(std::forward<T1>(v1), std::forward<T2>(v2));
}


// ==================================================================
// SLPairConst implementation

/** swap
Note that this function is a protected static member, not a friend function!
It doesn't make sense to provide a friend swap method for this class. 
The class is intended to be const, and shouldn't be used when the contents 
need to be changed. However, this function exists so a derived class can
swap the members.
@param[in] left  the pair to swap with right
@param[in] right the pair to swap with left
*/
template <typename KEY, typename VAL>
void SLPairConst<KEY, VAL>::memberSwap(SLPairConst& left, SLPairConst& right) noexcept
{
    using std::swap;
    swap(left.m_keyValPair, right.m_keyValPair);
}


/** default constructor
default-constructs KEY and VAL.
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
I went on the safe side and made the default constructor explicit too.
*/
template <typename KEY, typename VAL>
SLPairConst<KEY, VAL>::SLPairConst()
{ }


/** copy constructor
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to copy from
*/
template <typename KEY, typename VAL>
SLPairConst<KEY, VAL>::SLPairConst(const SLPairConst& other)
    : m_keyValPair(other.m_keyValPair)
{ }


/** move constructor
Instantiates key and value by moving other's key and value.
Other's key and value are not reassigned other than the move.
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to move. Leaves key and value in a post-moved state.
*/
template <typename KEY, typename VAL>
SLPairConst<KEY, VAL>::SLPairConst(SLPairConst&& other) noexcept
    : m_keyValPair(std::move(other.m_keyValPair))
{ }


/** argument constructor
@param[in] key Sets internal key (aka first)
@param[in] val Sets internal value (aka second)
*/
template <typename KEY, typename VAL>
template <typename FREF_KEY, typename FREF_VAL>
SLPairConst<KEY, VAL>::SLPairConst(FREF_KEY&& key, FREF_VAL&& val)
    : m_keyValPair(std::forward<FREF_KEY>(key), std::forward<FREF_VAL>(val))
{ }


/** pair conversion constructor
Converts an std::pair to an SLPairConst
@param[in] other The std::pair from which to copy .first and .second
*/
template <typename KEY, typename VAL>
SLPairConst<KEY, VAL>::SLPairConst(const std::pair<KEY, VAL>& other)
    : m_keyValPair(other)
{ }


/** pair conversion constructor
Converts an std::pair to an SLPairConst
@param[in] other The std::pair from which to move .first and .second
*/
template <typename KEY, typename VAL>
SLPairConst<KEY, VAL>::SLPairConst(std::pair<KEY, VAL>&& other) noexcept
    : m_keyValPair(std::move(other))
{ }


/** piecewise constructor
Pass in the tuples by calling std::forward_as_tuple.
@param[in] std::piecewise_construct_t tag used to specify this overload
@param[in] keyArgs Arguments, as a tuple, to forward to the key's constructor
@param[in] valArgs Arguments, as a tuple, to forward to the value's constructor
*/
template <typename KEY, typename VAL>
template <typename ...KEY_ARGS, typename ...VAL_ARGS>
SLPairConst<KEY, VAL>::SLPairConst(std::piecewise_construct_t, std::tuple<KEY_ARGS...> keyArgs, std::tuple<VAL_ARGS...> valArgs)
    : m_keyValPair(std::piecewise_construct, std::move(keyArgs), std::move(valArgs))
{ }


/** copy assignment
NOTE: The constructors and assignment overloads must be explicit so the 
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to copy-assign key and val from.
@return this
*/
template <typename KEY, typename VAL>
SLPairConst<KEY, VAL>& SLPairConst<KEY, VAL>::operator=(const SLPairConst& other)
{
    if (this != &other)
    {
        m_keyValPair = other.m_keyValPair;
    }
    return *this;
}


/** move assignment
Other's key and value are not necessarily reassigned other than the move.
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to move-assign key and val from.
@return this
*/
template <typename KEY, typename VAL>
SLPairConst<KEY, VAL>& SLPairConst<KEY, VAL>::operator=(SLPairConst&& other) noexcept
{
    m_keyValPair = std::move(other.m_keyValPair);
    return *this;
}


/** Gets a const reference to the key
@return a const reference to the key
*/
template <typename KEY, typename VAL>
const KEY& SLPairConst<KEY, VAL>::GetKey() const
{
    return first;
}


/** Gets a const reference to the value
@return a const reference to the value
*/
template <typename KEY, typename VAL>
const VAL& SLPairConst<KEY, VAL>::GetVal() const
{
    return second;
}


/** equality
Compares key. Only if key is equal will it compare val.
@param[in] right the right-hand pair of the comparison.
@return true if key and val are equal to other key and val.
*/
template <typename KEY, typename VAL>
bool SLPairConst<KEY, VAL>::operator==(const SLPairConst& right) const
{
    return m_keyValPair == right.m_keyValPair;
}


/** inequality
Compares key. Only if key is inequal will it compare val.
@param[in] right the right-hand pair of the comparison.
@return true if key and val are not equal to other key and val.
*/
template <typename KEY, typename VAL>
bool SLPairConst<KEY, VAL>::operator!=(const SLPairConst& right) const
{
    return m_keyValPair != right.m_keyValPair;
}


/** less than
Compares key. Only if keys are equivalent will it compare val.
Assumes items are equivalent if (!(A < B) && !(B < A))
@param[in] right the right-hand pair of the comparison.
@return true if key/val pair is lexographically less than other key/val pair.
*/
template <typename KEY, typename VAL>
bool SLPairConst<KEY, VAL>::operator<(const SLPairConst& right) const
{
    return m_keyValPair < right.m_keyValPair;
}


/** greater than
Compares key. Only if keys are equivalent will it compare val.
Assumes items are equivalent if (!(A > B) && !(B > A))
@param[in] right the right-hand pair of the comparison.
@return true if key/val pair is lexographically greater than other key/val pair.
*/
template <typename KEY, typename VAL>
bool SLPairConst<KEY, VAL>::operator>(const SLPairConst& right) const
{
    return m_keyValPair > right.m_keyValPair;
}


/** less than or equal
Compares key. Only if keys are equivalent will it compare val.
Assumes items are equivalent if ((A <= B) && (B <= A))
@param[in] right the right-hand pair of the comparison.
@return true if key/val pair is lexographically less than or equal to other key/val pair.
*/
template <typename KEY, typename VAL>
bool SLPairConst<KEY, VAL>::operator<=(const SLPairConst& right) const
{
    return m_keyValPair <= right.m_keyValPair;
}


/** greater than or equal
Compares key. Only if keys are equivalent will it compare val.
Assumes items are equivalent if ((A >= B) && (B >= A))
@param[in] right the right-hand pair of the comparison.
@return true if key/val pair is lexographically greater than or equal to other key/val pair.
*/
template <typename KEY, typename VAL>
bool SLPairConst<KEY, VAL>::operator>=(const SLPairConst& right) const
{
    return m_keyValPair >= right.m_keyValPair;
}


// ==================================================================
// SLPair implementation

/** default constructor
default-constructs KEY and VAL.
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
I went on the safe side and made the default constructor explicit too.
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>::SLPair()
{ }


/** copy constructor
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to copy from
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>::SLPair(const SLPair& other)
    : SLPairConst<KEY, VAL>(other)
{ }


/** move constructor
Instantiates key and value by moving other's key and value.
Other's key and value are not reassigned other than the move.
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to move. Leaves key and value in a post-moved state.
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>::SLPair(SLPair&& other) noexcept
    : SLPairConst<KEY, VAL>(std::move(other))
{ }


/** copy assignment
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to copy-assign key and val from.
@return this
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>& SLPair<KEY, VAL>::operator=(const SLPair& other)
{
    SLPairConst<KEY, VAL>::operator=(other);
    return *this;
}


/** move assignment
Other's key and value are not reassigned other than the move.
NOTE: The constructors and assignment overloads must be explicit so the
compiler knows how to set the references. You might not get a compiler error.
@param[in] other the pair to move-assign key and val from.
@return this
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>& SLPair<KEY, VAL>::operator=(SLPair&& other) noexcept
{
    SLPairConst<KEY, VAL>::operator=(std::move(other));
    return *this;
}


/** Member-wise swap of two pairs
friend function of SLPair, not a member function.
@param[in] left  the pair to swap with right
@param[in] right the pair to swap with left
*/
template <typename KEY, typename VAL>
void swap(SLPair<KEY, VAL>& left, SLPair<KEY, VAL>& right) noexcept
{
    SLPairConst<KEY, VAL>::memberSwap(left, right);
}


/** argument constructor
@param[in] key Sets internal key (aka first)
@param[in] val Sets internal value (aka second)
*/
template <typename KEY, typename VAL>
template <typename FREF_KEY, typename FREF_VAL>
SLPair<KEY, VAL>::SLPair(FREF_KEY&& key, FREF_VAL&& val)
    : SLPairConst<KEY, VAL>(std::forward<FREF_KEY>(key), std::forward<FREF_VAL>(val))
{ }


/** SLPairConst conversion constructor
We are unable to forward the base class constructors so we must provide this
@param[in] other the SLPairConst to copy
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>::SLPair(const SLPairConst<KEY, VAL>& other)
    : SLPairConst<KEY, VAL>(other)
{ }


/** pair conversion constructor
Converts an std::pair to an SLPair
@param[in] other The std::pair from which to copy .first and .second
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>::SLPair(const std::pair<KEY, VAL>& other)
    : SLPairConst<KEY, VAL>(other)
{ }


/** pair conversion constructor
Converts an std::pair to an SLPair
@param[in] other The std::pair from which to move .first and .second
*/
template <typename KEY, typename VAL>
SLPair<KEY, VAL>::SLPair(std::pair<KEY, VAL>&& other) noexcept
    : SLPairConst<KEY, VAL>(std::move(other))
{ }


/** piecewise constructor
Pass in the tuples by calling std::forward_as_tuple.
@param[in] std::piecewise_construct tag used to specify this overload
@param[in] keyArgs Arguments, as a tuple, to forward to the key's constructor
@param[in] valArgs Arguments, as a tuple, to forward to the value's constructor
*/
template <typename KEY, typename VAL>
template <typename ...KEY_ARGS, typename ...VAL_ARGS>
SLPair<KEY, VAL>::SLPair(std::piecewise_construct_t, std::tuple<KEY_ARGS...> keyArgs, std::tuple<VAL_ARGS...> valArgs)
    : SLPairConst<KEY, VAL>(std::piecewise_construct, std::move(keyArgs), std::move(valArgs))
{ }


/** Gets a reference to the value
@return a reference to the value
*/
template <typename KEY, typename VAL>
VAL& SLPair<KEY, VAL>::GetVal()
{
    return second;
}


/** Sets the value
@param[in] val the new assignment for val
*/
template <typename KEY, typename VAL>
void SLPair<KEY, VAL>::SetVal(VAL val)
{
    second = std::move(val);
}


}

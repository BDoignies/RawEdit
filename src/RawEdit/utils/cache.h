#pragma once

namespace RawEdit
{
    // We assume T to be light object
    // -  This is a cached method that allows to return a pointer 
    //    to the value. Faking the behaviour by returning proxy 
    //    type can only work if we do not expect functions taking 
    //    the typed pointer. 
    template<class T>
    struct Cached
    {
    public:
        Cached() : _value{}, _oldValue{} {}
        Cached(const T& val) : _value(val), _oldValue(val) {}

        Cached& operator=(const T& val) { _value = val; return *this; }

        bool dirty(bool consume = true) const 
        {
            const bool dirty = _value != _oldValue;
            if (consume && dirty) 
                _oldValue = _value;
            return dirty;
        }

        T& value()       { return _value; };
        T  value() const { return _value; };
    private:
        T _value;
        mutable T _oldValue;
    };
}

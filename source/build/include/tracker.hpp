template<typename T>
class TRACKER_NAME__
{
    public:
        T value;

        FORCE_INLINE T *operator&()
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return &this->value;
        }

        FORCE_INLINE T operator++()
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return ++this->value;
        }

        FORCE_INLINE T operator++(int)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value++;
        }

        FORCE_INLINE T operator--()
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return --this->value;
        }

        FORCE_INLINE T operator--(int)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value--;
        }

        template <typename U> FORCE_INLINE T operator=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value = (T)operand;
        }

        template <typename U> FORCE_INLINE T operator+=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value += (T)operand;
        }

        template <typename U> FORCE_INLINE T operator-=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value -= (T)operand;
        }

        template <typename U> FORCE_INLINE T operator*=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value *= (T)operand;
        }

        template <typename U> FORCE_INLINE T operator/=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value /= (T)operand;
        }

        template <typename U> FORCE_INLINE T operator|=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value |= (T)operand;
        }

        template <typename U> FORCE_INLINE T operator&=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value &= (T)operand;
        }

        template <typename U> FORCE_INLINE T operator^=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value ^= (T)operand;
        }

        template <typename U> FORCE_INLINE T operator<<=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value <<= (T)operand;
        }

        template <typename U> FORCE_INLINE T operator>>=(U operand)
        {
            TRACKER_HOOK_((intptr_t) & this->value);
            return this->value >>= (T)operand;
        }

        FORCE_INLINE operator T() const { return this->value; }

        FORCE_INLINE T cast() const { return this->value; }
};

template <typename T> struct is_signed<TRACKER_NAME__<T>>
{
    static constexpr bool value = std::is_signed<T>::value;
};
template <typename T> struct is_unsigned<TRACKER_NAME__<T>>
{
    static constexpr bool value = std::is_unsigned<T>::value;
};

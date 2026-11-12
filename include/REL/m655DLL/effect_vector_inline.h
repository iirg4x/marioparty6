#ifndef REL_M655DLL_EFFECT_VECTOR_INLINE_H
#define REL_M655DLL_EFFECT_VECTOR_INLINE_H
inline EffectVector &EffectVector::operator*=(f32 scale)
{
    PSVECScale(this, this, scale);
    return *this;
}

inline EffectVector &EffectVector::normalize()
{
    PSVECNormalize(this, this);
    return *this;
}
#endif

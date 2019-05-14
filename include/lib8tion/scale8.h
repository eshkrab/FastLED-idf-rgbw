#ifndef __INC_LIB8TION_SCALE_H
#define __INC_LIB8TION_SCALE_H

///@ingroup lib8tion

///@defgroup Scaling Scaling functions
/// Fast, efficient 8-bit scaling functions specifically
/// designed for high-performance LED programming.
///
/// Because of the AVR(Arduino) and ARM assembly language
/// implementations provided, using these functions often
/// results in smaller and faster code than the equivalent
/// program using plain "C" arithmetic and logic.
///@{

///  scale one byte by a second one, which is treated as
///  the numerator of a fraction whose denominator is 256
///  In other words, it computes i * (scale / 256)
///  4 clocks AVR with MUL, 2 clocks ARM
LIB8STATIC_ALWAYS_INLINE uint8_t scale8( uint8_t i, fract8 scale)
{
#if SCALE8_C == 1
#if (FASTLED_SCALE8_FIXED == 1)
    return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;
#else
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;
#endif
#endif
}


///  The "video" version of scale8 guarantees that the output will
///  be only be zero if one or both of the inputs are zero.  If both
///  inputs are non-zero, the output is guaranteed to be non-zero.
///  This makes for better 'video'/LED dimming, at the cost of
///  several additional cycles.
LIB8STATIC_ALWAYS_INLINE uint8_t scale8_video( uint8_t i, fract8 scale)
{
#if SCALE8_C == 1
    uint8_t j = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    return j;
#endif
}


/// This version of scale8 does not clean up the R1 register on AVR
/// If you are doing several 'scale8's in a row, use this, and
/// then explicitly call cleanup_R1.
LIB8STATIC_ALWAYS_INLINE uint8_t scale8_LEAVING_R1_DIRTY( uint8_t i, fract8 scale)
{
#if SCALE8_C == 1
#if (FASTLED_SCALE8_FIXED == 1)
    return (((uint16_t)i) * ((uint16_t)(scale)+1)) >> 8;
#else
    return ((int)i * (int)(scale) ) >> 8;
#endif
#endif
}

/// In place modifying version of scale8, also this version of nscale8 does not
/// clean up the R1 register on AVR
/// If you are doing several 'scale8's in a row, use this, and
/// then explicitly call cleanup_R1.

LIB8STATIC_ALWAYS_INLINE void nscale8_LEAVING_R1_DIRTY( uint8_t& i, fract8 scale)
{
#if SCALE8_C == 1
#if (FASTLED_SCALE8_FIXED == 1)
    i = (((uint16_t)i) * ((uint16_t)(scale)+1)) >> 8;
#else
    i = ((int)i * (int)(scale) ) >> 8;
#endif
#endif
}


/// This version of scale8_video does not clean up the R1 register on AVR
/// If you are doing several 'scale8_video's in a row, use this, and
/// then explicitly call cleanup_R1.
LIB8STATIC_ALWAYS_INLINE uint8_t scale8_video_LEAVING_R1_DIRTY( uint8_t i, fract8 scale)
{
#if SCALE8_C == 1
    uint8_t j = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    return j;
#endif
}

/// In place modifying version of scale8_video, also this version of nscale8_video
/// does not clean up the R1 register on AVR
/// If you are doing several 'scale8_video's in a row, use this, and
/// then explicitly call cleanup_R1.
LIB8STATIC_ALWAYS_INLINE void nscale8_video_LEAVING_R1_DIRTY( uint8_t & i, fract8 scale)
{
#if SCALE8_C == 1 
    i = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
#endif
}

/// Clean up the r1 register after a series of *LEAVING_R1_DIRTY calls
LIB8STATIC_ALWAYS_INLINE void cleanup_R1()
{
// #if CLEANUP_R1_AVRASM == 1
//     // Restore r1 to "0"; it's expected to always be that
//     asm volatile( "clr __zero_reg__  \n\t" : : : "r1" );
// #endif
}


/// scale three one byte values by a fourth one, which is treated as
///         the numerator of a fraction whose demominator is 256
///         In other words, it computes r,g,b * (scale / 256)
///
///         THIS FUNCTION ALWAYS MODIFIES ITS ARGUMENTS IN PLACE

LIB8STATIC void nscale8x3( uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w, fract8 scale)
{
#if SCALE8_C == 1
#if (FASTLED_SCALE8_FIXED == 1)
    uint16_t scale_fixed = scale + 1;
    r = (((uint16_t)r) * scale_fixed) >> 8;
    g = (((uint16_t)g) * scale_fixed) >> 8;
    b = (((uint16_t)b) * scale_fixed) >> 8;
    w = (((uint16_t)w) * scale_fixed) >> 8;
#else
    r = ((int)r * (int)(scale) ) >> 8;
    g = ((int)g * (int)(scale) ) >> 8;
    b = ((int)b * (int)(scale) ) >> 8;
    w = ((int)w * (int)(scale) ) >> 8;
#endif
#endif
}

/// scale three one byte values by a fourth one, which is treated as
///         the numerator of a fraction whose demominator is 256
///         In other words, it computes r,g,b * (scale / 256), ensuring
/// that non-zero values passed in remain non zero, no matter how low the scale
/// argument.
///
///         THIS FUNCTION ALWAYS MODIFIES ITS ARGUMENTS IN PLACE
LIB8STATIC void nscale8x3_video( uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w, fract8 scale)
{
#if SCALE8_C == 1
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    r = (r == 0) ? 0 : (((int)r * (int)(scale) ) >> 8) + nonzeroscale;
    g = (g == 0) ? 0 : (((int)g * (int)(scale) ) >> 8) + nonzeroscale;
    b = (b == 0) ? 0 : (((int)b * (int)(scale) ) >> 8) + nonzeroscale;
    w = (w == 0) ? 0 : (((int)w * (int)(scale) ) >> 8) + nonzeroscale;
#endif
}

///  scale two one byte values by a third one, which is treated as
///         the numerator of a fraction whose demominator is 256
///         In other words, it computes i,j * (scale / 256)
///
///         THIS FUNCTION ALWAYS MODIFIES ITS ARGUMENTS IN PLACE

LIB8STATIC void nscale8x2( uint8_t& i, uint8_t& j, fract8 scale)
{
#if SCALE8_C == 1
#if FASTLED_SCALE8_FIXED == 1
    uint16_t scale_fixed = scale + 1;
    i = (((uint16_t)i) * scale_fixed ) >> 8;
    j = (((uint16_t)j) * scale_fixed ) >> 8;
#else
    i = ((uint16_t)i * (uint16_t)(scale) ) >> 8;
    j = ((uint16_t)j * (uint16_t)(scale) ) >> 8;
#endif
#endif
}

///  scale two one byte values by a third one, which is treated as
///         the numerator of a fraction whose demominator is 256
///         In other words, it computes i,j * (scale / 256), ensuring
/// that non-zero values passed in remain non zero, no matter how low the scale
/// argument.
///
///         THIS FUNCTION ALWAYS MODIFIES ITS ARGUMENTS IN PLACE


LIB8STATIC void nscale8x2_video( uint8_t& i, uint8_t& j, fract8 scale)
{
#if SCALE8_C == 1
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    i = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    j = (j == 0) ? 0 : (((int)j * (int)(scale) ) >> 8) + nonzeroscale;
#endif
}


/// scale a 16-bit unsigned value by an 8-bit value,
///         considered as numerator of a fraction whose denominator
///         is 256. In other words, it computes i * (scale / 256)

LIB8STATIC_ALWAYS_INLINE uint16_t scale16by8( uint16_t i, fract8 scale )
{
#if SCALE16BY8_C == 1
    uint16_t result;
#if FASTLED_SCALE8_FIXED == 1
    result = (i * (1+((uint16_t)scale))) >> 8;
#else
    result = (i * scale) / 256;
#endif
    return result;
#endif
}

/// scale a 16-bit unsigned value by a 16-bit value,
///         considered as numerator of a fraction whose denominator
///         is 65536. In other words, it computes i * (scale / 65536)

LIB8STATIC uint16_t scale16( uint16_t i, fract16 scale )
{
#if SCALE16_C == 1
    uint16_t result;
#if FASTLED_SCALE8_FIXED == 1
    result = ((uint32_t)(i) * (1+(uint32_t)(scale))) / 65536;
#else
    result = ((uint32_t)(i) * (uint32_t)(scale)) / 65536;
#endif
    return result;
#endif
}
///@}

///@defgroup Dimming Dimming and brightening functions
///
/// Dimming and brightening functions
///
/// The eye does not respond in a linear way to light.
/// High speed PWM'd LEDs at 50% duty cycle appear far
/// brighter then the 'half as bright' you might expect.
///
/// If you want your midpoint brightness leve (128) to
/// appear half as bright as 'full' brightness (255), you
/// have to apply a 'dimming function'.
///@{

/// Adjust a scaling value for dimming
LIB8STATIC uint8_t dim8_raw( uint8_t x)
{
    return scale8( x, x);
}

/// Adjust a scaling value for dimming for video (value will never go below 1)
LIB8STATIC uint8_t dim8_video( uint8_t x)
{
    return scale8_video( x, x);
}

/// Linear version of the dimming function that halves for values < 128
LIB8STATIC uint8_t dim8_lin( uint8_t x )
{
    if( x & 0x80 ) {
        x = scale8( x, x);
    } else {
        x += 1;
        x /= 2;
    }
    return x;
}

/// inverse of the dimming function, brighten a value
LIB8STATIC uint8_t brighten8_raw( uint8_t x)
{
    uint8_t ix = 255 - x;
    return 255 - scale8( ix, ix);
}

/// inverse of the dimming function, brighten a value
LIB8STATIC uint8_t brighten8_video( uint8_t x)
{
    uint8_t ix = 255 - x;
    return 255 - scale8_video( ix, ix);
}

/// inverse of the dimming function, brighten a value
LIB8STATIC uint8_t brighten8_lin( uint8_t x )
{
    uint8_t ix = 255 - x;
    if( ix & 0x80 ) {
        ix = scale8( ix, ix);
    } else {
        ix += 1;
        ix /= 2;
    }
    return 255 - ix;
}

///@}
#endif

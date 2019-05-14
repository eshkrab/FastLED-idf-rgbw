#ifndef __INC_COLORUTILS_H
#define __INC_COLORUTILS_H

///@file colorutils.h
/// functions for color fill, paletters, blending, and more

#include "FastLED.h"
#include "pixeltypes.h"
#include "fastled_progmem.h"

FASTLED_NAMESPACE_BEGIN
///@defgroup Colorutils Color utility functions
///A variety of functions for working with color, palletes, and leds
///@{

/// fill_solid -   fill a range of LEDs with a solid color
///                Example: fill_solid( leds, NUM_LEDS, CRGBW(50,0,200));
void fill_solid( struct CRGBW * leds, int numToFill,
                 const struct CRGBW& color);

/// fill_solid -   fill a range of LEDs with a solid color
///                Example: fill_solid( leds, NUM_LEDS, CRGBW(50,0,200));
void fill_solid( struct CHSV* targetArray, int numToFill,
				 const struct CHSV& hsvColor);


/// fill_rainbow - fill a range of LEDs with a rainbow of colors, at
///                full saturation and full value (brightness)
void fill_rainbow( struct CRGBW * pFirstLED, int numToFill,
                   uint8_t initialhue,
                   uint8_t deltahue = 5);

/// fill_rainbow - fill a range of LEDs with a rainbow of colors, at
///                full saturation and full value (brightness)
void fill_rainbow( struct CHSV * targetArray, int numToFill,
                   uint8_t initialhue,
                   uint8_t deltahue = 5);


// fill_gradient - fill an array of colors with a smooth HSV gradient
//                 between two specified HSV colors.
//                 Since 'hue' is a value around a color wheel,
//                 there are always two ways to sweep from one hue
//                 to another.
//                 This function lets you specify which way you want
//                 the hue gradient to sweep around the color wheel:
//                   FORWARD_HUES: hue always goes clockwise
//                   BACKWARD_HUES: hue always goes counter-clockwise
//                   SHORTEST_HUES: hue goes whichever way is shortest
//                   LONGEST_HUES: hue goes whichever way is longest
//                 The default is SHORTEST_HUES, as this is nearly
//                 always what is wanted.
//
// fill_gradient can write the gradient colors EITHER
//     (1) into an array of CRGBWs (e.g., into leds[] array, or an RGB Palette)
//   OR
//     (2) into an array of CHSVs (e.g. an HSV Palette).
//
//   In the case of writing into a CRGBW array, the gradient is
//   computed in HSV space, and then HSV values are converted to RGB
//   as they're written into the RGB array.

typedef enum { FORWARD_HUES, BACKWARD_HUES, SHORTEST_HUES, LONGEST_HUES } TGradientDirectionCode;



#define saccum87 int16_t

/// fill_gradient - fill an array of colors with a smooth HSV gradient
/// between two specified HSV colors.
/// Since 'hue' is a value around a color wheel,
/// there are always two ways to sweep from one hue
/// to another.
/// This function lets you specify which way you want
/// the hue gradient to sweep around the color wheel:
///
///     FORWARD_HUES: hue always goes clockwise
///     BACKWARD_HUES: hue always goes counter-clockwise
///     SHORTEST_HUES: hue goes whichever way is shortest
///     LONGEST_HUES: hue goes whichever way is longest
///
/// The default is SHORTEST_HUES, as this is nearly
/// always what is wanted.
///
/// fill_gradient can write the gradient colors EITHER
///     (1) into an array of CRGBWs (e.g., into leds[] array, or an RGB Palette)
///   OR
///     (2) into an array of CHSVs (e.g. an HSV Palette).
///
///   In the case of writing into a CRGBW array, the gradient is
///   computed in HSV space, and then HSV values are converted to RGB
///   as they're written into the RGB array.
template <typename T>
void fill_gradient( T* targetArray,
                    uint16_t startpos, CHSV startcolor,
                    uint16_t endpos,   CHSV endcolor,
                    TGradientDirectionCode directionCode  = SHORTEST_HUES )
{
    // if the points are in the wrong order, straighten them
    if( endpos < startpos ) {
        uint16_t t = endpos;
        CHSV tc = endcolor;
        endcolor = startcolor;
        endpos = startpos;
        startpos = t;
        startcolor = tc;
    }

    // If we're fading toward black (val=0) or white (sat=0),
    // then set the endhue to the starthue.
    // This lets us ramp smoothly to black or white, regardless
    // of what 'hue' was set in the endcolor (since it doesn't matter)
    if( endcolor.value == 0 || endcolor.saturation == 0) {
        endcolor.hue = startcolor.hue;
    }

    // Similarly, if we're fading in from black (val=0) or white (sat=0)
    // then set the starthue to the endhue.
    // This lets us ramp smoothly up from black or white, regardless
    // of what 'hue' was set in the startcolor (since it doesn't matter)
    if( startcolor.value == 0 || startcolor.saturation == 0) {
        startcolor.hue = endcolor.hue;
    }

    saccum87 huedistance87;
    saccum87 satdistance87;
    saccum87 valdistance87;

    satdistance87 = (endcolor.sat - startcolor.sat) << 7;
    valdistance87 = (endcolor.val - startcolor.val) << 7;

    uint8_t huedelta8 = endcolor.hue - startcolor.hue;

    if( directionCode == SHORTEST_HUES ) {
        directionCode = FORWARD_HUES;
        if( huedelta8 > 127) {
            directionCode = BACKWARD_HUES;
        }
    }

    if( directionCode == LONGEST_HUES ) {
        directionCode = FORWARD_HUES;
        if( huedelta8 < 128) {
            directionCode = BACKWARD_HUES;
        }
    }

    if( directionCode == FORWARD_HUES) {
        huedistance87 = huedelta8 << 7;
    }
    else /* directionCode == BACKWARD_HUES */
    {
        huedistance87 = (uint8_t)(256 - huedelta8) << 7;
        huedistance87 = -huedistance87;
    }

    uint16_t pixeldistance = endpos - startpos;
    int16_t divisor = pixeldistance ? pixeldistance : 1;

    saccum87 huedelta87 = huedistance87 / divisor;
    saccum87 satdelta87 = satdistance87 / divisor;
    saccum87 valdelta87 = valdistance87 / divisor;

    huedelta87 *= 2;
    satdelta87 *= 2;
    valdelta87 *= 2;

    accum88 hue88 = startcolor.hue << 8;
    accum88 sat88 = startcolor.sat << 8;
    accum88 val88 = startcolor.val << 8;
    for( uint16_t i = startpos; i <= endpos; i++) {
        targetArray[i] = CHSV( hue88 >> 8, sat88 >> 8, val88 >> 8);
        hue88 += huedelta87;
        sat88 += satdelta87;
        val88 += valdelta87;
    }
}


// Convenience functions to fill an array of colors with a
// two-color, three-color, or four-color gradient
template <typename T>
void fill_gradient( T* targetArray, uint16_t numLeds, const CHSV& c1, const CHSV& c2,
					TGradientDirectionCode directionCode = SHORTEST_HUES )
{
    uint16_t last = numLeds - 1;
    fill_gradient( targetArray, 0, c1, last, c2, directionCode);
}

template <typename T>
void fill_gradient( T* targetArray, uint16_t numLeds,
					const CHSV& c1, const CHSV& c2, const CHSV& c3,
					TGradientDirectionCode directionCode = SHORTEST_HUES )
{
    uint16_t half = (numLeds / 2);
    uint16_t last = numLeds - 1;
    fill_gradient( targetArray,    0, c1, half, c2, directionCode);
    fill_gradient( targetArray, half, c2, last, c3, directionCode);
}

template <typename T>
void fill_gradient( T* targetArray, uint16_t numLeds,
					const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4,
					TGradientDirectionCode directionCode = SHORTEST_HUES )
{
    uint16_t onethird = (numLeds / 3);
    uint16_t twothirds = ((numLeds * 2) / 3);
    uint16_t last = numLeds - 1;
    fill_gradient( targetArray,         0, c1,  onethird, c2, directionCode);
    fill_gradient( targetArray,  onethird, c2, twothirds, c3, directionCode);
    fill_gradient( targetArray, twothirds, c3,      last, c4, directionCode);
}

// convenience synonym
#define fill_gradient_HSV fill_gradient


// fill_gradient_RGB - fill a range of LEDs with a smooth RGB gradient
//                     between two specified RGB colors.
//                     Unlike HSV, there is no 'color wheel' in RGB space,
//                     and therefore there's only one 'direction' for the
//                     gradient to go, and no 'direction code' is needed.
void fill_gradient_RGB( CRGBW* leds,
                       uint16_t startpos, CRGBW startcolor,
                       uint16_t endpos,   CRGBW endcolor );
void fill_gradient_RGB( CRGBW* leds, uint16_t numLeds, const CRGBW& c1, const CRGBW& c2);
void fill_gradient_RGB( CRGBW* leds, uint16_t numLeds, const CRGBW& c1, const CRGBW& c2, const CRGBW& c3);
void fill_gradient_RGB( CRGBW* leds, uint16_t numLeds, const CRGBW& c1, const CRGBW& c2, const CRGBW& c3, const CRGBW& c4);


// fadeLightBy and fade_video - reduce the brightness of an array
//                              of pixels all at once.  Guaranteed
//                              to never fade all the way to black.
//                              (The two names are synonyms.)
void fadeLightBy(   CRGBW* leds, uint16_t num_leds, uint8_t fadeBy);
void fade_video(    CRGBW* leds, uint16_t num_leds, uint8_t fadeBy);

// nscale8_video - scale down the brightness of an array of pixels
//                 all at once.  Guaranteed to never scale a pixel
//                 all the way down to black, unless 'scale' is zero.
void nscale8_video( CRGBW* leds, uint16_t num_leds, uint8_t scale);

// fadeToBlackBy and fade_raw - reduce the brightness of an array
//                              of pixels all at once.  These
//                              functions will eventually fade all
//                              the way to black.
//                              (The two names are synonyms.)
void fadeToBlackBy( CRGBW* leds, uint16_t num_leds, uint8_t fadeBy);
void fade_raw(      CRGBW* leds, uint16_t num_leds, uint8_t fadeBy);

// nscale8 - scale down the brightness of an array of pixels
//           all at once.  This function can scale pixels all the
//           way down to black even if 'scale' is not zero.
void nscale8(       CRGBW* leds, uint16_t num_leds, uint8_t scale);

// fadeUsingColor - scale down the brightness of an array of pixels,
//                  as though it were seen through a transparent
//                  filter with the specified color.
//                  For example, if the colormask is
//                    CRGBW( 200, 100, 50)
//                  then the pixels' red will be faded to 200/256ths,
//                  their green to 100/256ths, and their blue to 50/256ths.
//                  This particular example give a 'hot fade' look,
//                  with white fading to yellow, then red, then black.
//                  You can also use colormasks like CRGBW::Blue to
//                  zero out the red and green elements, leaving blue
//                  (largely) the same.
void fadeUsingColor( CRGBW* leds, uint16_t numLeds, const CRGBW& colormask);


// Pixel blending
//
// blend - computes a new color blended some fraction of the way
//         between two other colors.
CRGBW  blend( const CRGBW& p1, const CRGBW& p2, fract8 amountOfP2 );

CHSV  blend( const CHSV& p1, const CHSV& p2, fract8 amountOfP2,
            TGradientDirectionCode directionCode = SHORTEST_HUES );

// blend - computes a new color blended array of colors, each
//         a given fraction of the way between corresponding
//         elements of two source arrays of colors.
//         Useful for blending palettes.
CRGBW* blend( const CRGBW* src1, const CRGBW* src2, CRGBW* dest,
             uint16_t count, fract8 amountOfsrc2 );

CHSV* blend( const CHSV* src1, const CHSV* src2, CHSV* dest,
            uint16_t count, fract8 amountOfsrc2,
            TGradientDirectionCode directionCode = SHORTEST_HUES );

// nblend - destructively modifies one color, blending
//          in a given fraction of an overlay color
CRGBW& nblend( CRGBW& existing, const CRGBW& overlay, fract8 amountOfOverlay );

CHSV& nblend( CHSV& existing, const CHSV& overlay, fract8 amountOfOverlay,
             TGradientDirectionCode directionCode = SHORTEST_HUES );

// nblend - destructively blends a given fraction of
//          a new color array into an existing color array
void  nblend( CRGBW* existing, CRGBW* overlay, uint16_t count, fract8 amountOfOverlay);

void  nblend( CHSV* existing, CHSV* overlay, uint16_t count, fract8 amountOfOverlay,
             TGradientDirectionCode directionCode = SHORTEST_HUES);


// blur1d: one-dimensional blur filter. Spreads light to 2 line neighbors.
// blur2d: two-dimensional blur filter. Spreads light to 8 XY neighbors.
//
//           0 = no spread at all
//          64 = moderate spreading
//         172 = maximum smooth, even spreading
//
//         173..255 = wider spreading, but increasing flicker
//
//         Total light is NOT entirely conserved, so many repeated
//         calls to 'blur' will also result in the light fading,
//         eventually all the way to black; this is by design so that
//         it can be used to (slowly) clear the LEDs to black.
void blur1d( CRGBW* leds, uint16_t numLeds, fract8 blur_amount);
void blur2d( CRGBW* leds, uint8_t width, uint8_t height, fract8 blur_amount);

// blurRows: perform a blur1d on every row of a rectangular matrix
void blurRows( CRGBW* leds, uint8_t width, uint8_t height, fract8 blur_amount);
// blurColumns: perform a blur1d on each column of a rectangular matrix
void blurColumns(CRGBW* leds, uint8_t width, uint8_t height, fract8 blur_amount);


// CRGBW HeatColor( uint8_t temperature)
//
// Approximates a 'black body radiation' spectrum for
// a given 'heat' level.  This is useful for animations of 'fire'.
// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
// This is NOT a chromatically correct 'black body radiation'
// spectrum, but it's surprisingly close, and it's fast and small.
CRGBW HeatColor( uint8_t temperature);


// Palettes
//
// RGB Palettes map an 8-bit value (0..255) to an RGB color.
//
// You can create any color palette you wish; a couple of starters
// are provided: Forest, Clouds, Lava, Ocean, Rainbow, and Rainbow Stripes.
//
// Palettes come in the traditional 256-entry variety, which take
// up 768 bytes of RAM, and lightweight 16-entry varieties.  The 16-entry
// variety automatically interpolates between its entries to produce
// a full 256-element color map, but at a cost of only 48 bytes or RAM.
//
// Basic operation is like this: (example shows the 16-entry variety)
// 1. Declare your palette storage:
//    CRGBWPalette16 myPalette;
//
// 2. Fill myPalette with your own 16 colors, or with a preset color scheme.
//    You can specify your 16 colors a variety of ways:
//      CRGBWPalette16 myPalette(
//          CRGBW::Black,
//          CRGBW::Black,
//          CRGBW::Red,
//          CRGBW::Yellow,
//          CRGBW::Green,
//          CRGBW::Blue,
//          CRGBW::Purple,
//          CRGBW::Black,
//
//          0x100000,
//          0x200000,
//          0x400000,
//          0x800000,
//
//          CHSV( 30,255,255),
//          CHSV( 50,255,255),
//          CHSV( 70,255,255),
//          CHSV( 90,255,255)
//      );
//
//    Or you can initiaize your palette with a preset color scheme:
//      myPalette = RainbowStripesColors_p;
//
// 3. Any time you want to set a pixel to a color from your palette, use
//    "ColorFromPalette(...)" as shown:
//
//      uint8_t index = /* any value 0..255 */;
//      leds[i] = ColorFromPalette( myPalette, index);
//
//    Even though your palette has only 16 explicily defined entries, you
//    can use an 'index' from 0..255.  The 16 explicit palette entries will
//    be spread evenly across the 0..255 range, and the intermedate values
//    will be RGB-interpolated between adjacent explicit entries.
//
//    It's easier to use than it sounds.
//

class CRGBWPalette16;
class CRGBWPalette32;
class CRGBWPalette256;
class CHSVPalette16;
class CHSVPalette32;
class CHSVPalette256;
typedef uint32_t TProgmemRGBPalette16[16];
typedef uint32_t TProgmemHSVPalette16[16];
#define TProgmemPalette16 TProgmemRGBPalette16
typedef uint32_t TProgmemRGBPalette32[32];
typedef uint32_t TProgmemHSVPalette32[32];
#define TProgmemPalette32 TProgmemRGBPalette32

typedef const uint8_t TProgmemRGBGradientPalette_byte ;
typedef const TProgmemRGBGradientPalette_byte *TProgmemRGBGradientPalette_bytes;
typedef TProgmemRGBGradientPalette_bytes TProgmemRGBGradientPalettePtr;
typedef union {
    struct {
        uint8_t index;
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t w;
    };
    uint32_t dword;
    uint8_t  bytes[4];
} TRGBGradientPaletteEntryUnion;

typedef uint8_t TDynamicRGBGradientPalette_byte ;
typedef const TDynamicRGBGradientPalette_byte *TDynamicRGBGradientPalette_bytes;
typedef TDynamicRGBGradientPalette_bytes TDynamicRGBGradientPalettePtr;

// Convert a 16-entry palette to a 256-entry palette
void UpscalePalette(const struct CRGBWPalette16& srcpal16, struct CRGBWPalette256& destpal256);
void UpscalePalette(const struct CHSVPalette16& srcpal16, struct CHSVPalette256& destpal256);

// Convert a 16-entry palette to a 32-entry palette
void UpscalePalette(const struct CRGBWPalette16& srcpal16, struct CRGBWPalette32& destpal32);
void UpscalePalette(const struct CHSVPalette16& srcpal16, struct CHSVPalette32& destpal32);

// Convert a 32-entry palette to a 256-entry palette
void UpscalePalette(const struct CRGBWPalette32& srcpal32, struct CRGBWPalette256& destpal256);
void UpscalePalette(const struct CHSVPalette32& srcpal32, struct CHSVPalette256& destpal256);


class CHSVPalette16 {
public:
    CHSV entries[16];
    CHSVPalette16() {};
    CHSVPalette16( const CHSV& c00,const CHSV& c01,const CHSV& c02,const CHSV& c03,
                    const CHSV& c04,const CHSV& c05,const CHSV& c06,const CHSV& c07,
                    const CHSV& c08,const CHSV& c09,const CHSV& c10,const CHSV& c11,
                    const CHSV& c12,const CHSV& c13,const CHSV& c14,const CHSV& c15 )
    {
        entries[0]=c00; entries[1]=c01; entries[2]=c02; entries[3]=c03;
        entries[4]=c04; entries[5]=c05; entries[6]=c06; entries[7]=c07;
        entries[8]=c08; entries[9]=c09; entries[10]=c10; entries[11]=c11;
        entries[12]=c12; entries[13]=c13; entries[14]=c14; entries[15]=c15;
    };

    CHSVPalette16( const CHSVPalette16& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CHSVPalette16& operator=( const CHSVPalette16& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }

    CHSVPalette16( const TProgmemHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; i++) {
            CRGBW xyz   =  FL_PGM_READ_DWORD_NEAR( rhs + i);
            entries[i].hue = xyz.red;
            entries[i].sat = xyz.green;
            entries[i].val = xyz.blue;
        }
    }
    CHSVPalette16& operator=( const TProgmemHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; i++) {
            CRGBW xyz   =  FL_PGM_READ_DWORD_NEAR( rhs + i);
            entries[i].hue = xyz.red;
            entries[i].sat = xyz.green;
            entries[i].val = xyz.blue;
        }
        return *this;
    }

    inline CHSV& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CHSV& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }

    inline CHSV& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CHSV& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }

    operator CHSV*()
    {
        return &(entries[0]);
    }

    bool operator==( const CHSVPalette16 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint8_t i = 0; i < (sizeof( entries)); i++) {
            if( *p != *q) return false;
            p++;
            q++;
        }
        return true;
    }
    bool operator!=( const CHSVPalette16 rhs)
    {
        return !( *this == rhs);
    }
    
    CHSVPalette16( const CHSV& c1)
    {
        fill_solid( &(entries[0]), 16, c1);
    }
    CHSVPalette16( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 16, c1, c2);
    }
    CHSVPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3);
    }
    CHSVPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3, c4);
    }

};

class CHSVPalette256 {
public:
    CHSV entries[256];
    CHSVPalette256() {};
    CHSVPalette256( const CHSV& c00,const CHSV& c01,const CHSV& c02,const CHSV& c03,
                  const CHSV& c04,const CHSV& c05,const CHSV& c06,const CHSV& c07,
                  const CHSV& c08,const CHSV& c09,const CHSV& c10,const CHSV& c11,
                  const CHSV& c12,const CHSV& c13,const CHSV& c14,const CHSV& c15 )
    {
        CHSVPalette16 p16(c00,c01,c02,c03,c04,c05,c06,c07,
                          c08,c09,c10,c11,c12,c13,c14,c15);
        *this = p16;
    };

    CHSVPalette256( const CHSVPalette256& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CHSVPalette256& operator=( const CHSVPalette256& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }

    CHSVPalette256( const CHSVPalette16& rhs16)
    {
        UpscalePalette( rhs16, *this);
    }
    CHSVPalette256& operator=( const CHSVPalette16& rhs16)
    {
        UpscalePalette( rhs16, *this);
        return *this;
    }

    CHSVPalette256( const TProgmemRGBPalette16& rhs)
    {
        CHSVPalette16 p16(rhs);
        *this = p16;
    }
    CHSVPalette256& operator=( const TProgmemRGBPalette16& rhs)
    {
        CHSVPalette16 p16(rhs);
        *this = p16;
        return *this;
    }

    inline CHSV& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CHSV& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }

    inline CHSV& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CHSV& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }

    operator CHSV*()
    {
        return &(entries[0]);
    }

    bool operator==( const CHSVPalette256 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint16_t i = 0; i < (sizeof( entries)); i++) {
            if( *p != *q) return false;
            p++;
            q++;
        }
        return true;
    }
    bool operator!=( const CHSVPalette256 rhs)
    {
        return !( *this == rhs);
    }
    
    CHSVPalette256( const CHSV& c1)
    {
      fill_solid( &(entries[0]), 256, c1);
    }
    CHSVPalette256( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 256, c1, c2);
    }
    CHSVPalette256( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 256, c1, c2, c3);
    }
    CHSVPalette256( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 256, c1, c2, c3, c4);
    }
};

class CRGBWPalette16 {
public:
    CRGBW entries[16];
    CRGBWPalette16() {};
    CRGBWPalette16( const CRGBW& c00,const CRGBW& c01,const CRGBW& c02,const CRGBW& c03,
                    const CRGBW& c04,const CRGBW& c05,const CRGBW& c06,const CRGBW& c07,
                    const CRGBW& c08,const CRGBW& c09,const CRGBW& c10,const CRGBW& c11,
                    const CRGBW& c12,const CRGBW& c13,const CRGBW& c14,const CRGBW& c15 )
    {
        entries[0]=c00; entries[1]=c01; entries[2]=c02; entries[3]=c03;
        entries[4]=c04; entries[5]=c05; entries[6]=c06; entries[7]=c07;
        entries[8]=c08; entries[9]=c09; entries[10]=c10; entries[11]=c11;
        entries[12]=c12; entries[13]=c13; entries[14]=c14; entries[15]=c15;
    };

    CRGBWPalette16( const CRGBWPalette16& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CRGBWPalette16( const CRGBW rhs[16])
    {
        memmove8( &(entries[0]), &(rhs[0]), sizeof( entries));
    }
    CRGBWPalette16& operator=( const CRGBWPalette16& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }
    CRGBWPalette16& operator=( const CRGBW rhs[16])
    {
        memmove8( &(entries[0]), &(rhs[0]), sizeof( entries));
        return *this;
    }

    CRGBWPalette16( const CHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; i++) {
    		entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
        }
    }
    CRGBWPalette16( const CHSV rhs[16])
    {
        for( uint8_t i = 0; i < 16; i++) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
    }
    CRGBWPalette16& operator=( const CHSVPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; i++) {
    		entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
        }
        return *this;
    }
    CRGBWPalette16& operator=( const CHSV rhs[16])
    {
        for( uint8_t i = 0; i < 16; i++) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
        return *this;
    }

    CRGBWPalette16( const TProgmemRGBPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; i++) {
            entries[i] =  FL_PGM_READ_DWORD_NEAR( rhs + i);
        }
    }
    CRGBWPalette16& operator=( const TProgmemRGBPalette16& rhs)
    {
        for( uint8_t i = 0; i < 16; i++) {
            entries[i] =  FL_PGM_READ_DWORD_NEAR( rhs + i);
        }
        return *this;
    }

    bool operator==( const CRGBWPalette16 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint8_t i = 0; i < (sizeof( entries)); i++) {
            if( *p != *q) return false;
            p++;
            q++;
        }
        return true;
    }
    bool operator!=( const CRGBWPalette16 rhs)
    {
        return !( *this == rhs);
    }
    
    inline CRGBW& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CRGBW& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }

    inline CRGBW& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CRGBW& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }

    operator CRGBW*()
    {
        return &(entries[0]);
    }

    CRGBWPalette16( const CHSV& c1)
    {
        fill_solid( &(entries[0]), 16, c1);
    }
    CRGBWPalette16( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 16, c1, c2);
    }
    CRGBWPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3);
    }
    CRGBWPalette16( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 16, c1, c2, c3, c4);
    }

    CRGBWPalette16( const CRGBW& c1)
    {
        fill_solid( &(entries[0]), 16, c1);
    }
    CRGBWPalette16( const CRGBW& c1, const CRGBW& c2)
    {
        fill_gradient_RGB( &(entries[0]), 16, c1, c2);
    }
    CRGBWPalette16( const CRGBW& c1, const CRGBW& c2, const CRGBW& c3)
    {
        fill_gradient_RGB( &(entries[0]), 16, c1, c2, c3);
    }
    CRGBWPalette16( const CRGBW& c1, const CRGBW& c2, const CRGBW& c3, const CRGBW& c4)
    {
        fill_gradient_RGB( &(entries[0]), 16, c1, c2, c3, c4);
    }


    // Gradient palettes are loaded into CRGBW16Palettes in such a way
    // that, if possible, every color represented in the gradient palette
    // is also represented in the CRGBWPalette16.
    // For example, consider a gradient palette that is all black except
    // for a single, one-element-wide (1/256th!) spike of red in the middle:
    //     0,   0,0,0
    //   124,   0,0,0
    //   125, 255,0,0  // one 1/256th-palette-wide red stripe
    //   126,   0,0,0
    //   255,   0,0,0
    // A naive conversion of this 256-element palette to a 16-element palette
    // might accidentally completely eliminate the red spike, rendering the
    // palette completely black.
    // However, the conversions provided here would attempt to include a
    // the red stripe in the output, more-or-less as faithfully as possible.
    // So in this case, the resulting CRGBWPalette16 palette would have a red
    // stripe in the middle which was 1/16th of a palette wide -- the
    // narrowest possible in a CRGBWPalette16.
    // This means that the relative width of stripes in a CRGBWPalette16
    // will be, by definition, different from the widths in the gradient
    // palette.  This code attempts to preserve "all the colors", rather than
    // the exact stripe widths at the expense of dropping some colors.
    CRGBWPalette16( TProgmemRGBGradientPalette_bytes progpal )
    {
        *this = progpal;
    }
    CRGBWPalette16& operator=( TProgmemRGBGradientPalette_bytes progpal )
    {
        TRGBGradientPaletteEntryUnion* progent = (TRGBGradientPaletteEntryUnion*)(progpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u.dword = FL_PGM_READ_DWORD_NEAR(progent + count);
            count++;;
        } while ( u.index != 255);

        int8_t lastSlotUsed = -1;

        u.dword = FL_PGM_READ_DWORD_NEAR( progent);
        CRGBW rgbstart( u.r, u.g, u.b, u.w);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while( indexstart < 255) {
            progent++;
            u.dword = FL_PGM_READ_DWORD_NEAR( progent);
            int indexend  = u.index;
            CRGBW rgbend( u.r, u.g, u.b, u.w);
            istart8 = indexstart / 16;
            iend8   = indexend   / 16;
            if( count < 16) {
                if( (istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
                    istart8 = lastSlotUsed + 1;
                    if( iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB( &(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
    CRGBWPalette16& loadDynamicGradientPalette( TDynamicRGBGradientPalette_bytes gpal )
    {
        TRGBGradientPaletteEntryUnion* ent = (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u = *(ent + count);
            count++;;
        } while ( u.index != 255);

        int8_t lastSlotUsed = -1;


        u = *ent;
        CRGBW rgbstart( u.r, u.g, u.b, u.w);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while( indexstart < 255) {
            ent++;
            u = *ent;
            int indexend  = u.index;
            CRGBW rgbend( u.r, u.g, u.b, u.w);
            istart8 = indexstart / 16;
            iend8   = indexend   / 16;
            if( count < 16) {
                if( (istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
                    istart8 = lastSlotUsed + 1;
                    if( iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB( &(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }

};



class CHSVPalette32 {
public:
    CHSV entries[32];
    CHSVPalette32() {};
    CHSVPalette32( const CHSV& c00,const CHSV& c01,const CHSV& c02,const CHSV& c03,
                  const CHSV& c04,const CHSV& c05,const CHSV& c06,const CHSV& c07,
                  const CHSV& c08,const CHSV& c09,const CHSV& c10,const CHSV& c11,
                  const CHSV& c12,const CHSV& c13,const CHSV& c14,const CHSV& c15 )
    {
        for( uint8_t i = 0; i < 2; i++) {
            entries[0+i]=c00; entries[2+i]=c01; entries[4+i]=c02; entries[6+i]=c03;
            entries[8+i]=c04; entries[10+i]=c05; entries[12+i]=c06; entries[14+i]=c07;
            entries[16+i]=c08; entries[18+i]=c09; entries[20+i]=c10; entries[22+i]=c11;
            entries[24+i]=c12; entries[26+i]=c13; entries[28+i]=c14; entries[30+i]=c15;
        }
    };
    
    CHSVPalette32( const CHSVPalette32& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CHSVPalette32& operator=( const CHSVPalette32& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }
    
    CHSVPalette32( const TProgmemHSVPalette32& rhs)
    {
        for( uint8_t i = 0; i < 32; i++) {
            CRGBW xyz   =  FL_PGM_READ_DWORD_NEAR( rhs + i);
            entries[i].hue = xyz.red;
            entries[i].sat = xyz.green;
            entries[i].val = xyz.blue;
        }
    }
    CHSVPalette32& operator=( const TProgmemHSVPalette32& rhs)
    {
        for( uint8_t i = 0; i < 32; i++) {
            CRGBW xyz   =  FL_PGM_READ_DWORD_NEAR( rhs + i);
            entries[i].hue = xyz.red;
            entries[i].sat = xyz.green;
            entries[i].val = xyz.blue;
        }
        return *this;
    }
    
    inline CHSV& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CHSV& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }
    
    inline CHSV& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CHSV& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    
    operator CHSV*()
    {
        return &(entries[0]);
    }
    
    bool operator==( const CHSVPalette32 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint8_t i = 0; i < (sizeof( entries)); i++) {
            if( *p != *q) return false;
            p++;
            q++;
        }
        return true;
    }
    bool operator!=( const CHSVPalette32 rhs)
    {
        return !( *this == rhs);
    }
    
    CHSVPalette32( const CHSV& c1)
    {
        fill_solid( &(entries[0]), 32, c1);
    }
    CHSVPalette32( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 32, c1, c2);
    }
    CHSVPalette32( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 32, c1, c2, c3);
    }
    CHSVPalette32( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 32, c1, c2, c3, c4);
    }
    
};

class CRGBWPalette32 {
public:
    CRGBW entries[32];
    CRGBWPalette32() {};
    CRGBWPalette32( const CRGBW& c00,const CRGBW& c01,const CRGBW& c02,const CRGBW& c03,
                  const CRGBW& c04,const CRGBW& c05,const CRGBW& c06,const CRGBW& c07,
                  const CRGBW& c08,const CRGBW& c09,const CRGBW& c10,const CRGBW& c11,
                  const CRGBW& c12,const CRGBW& c13,const CRGBW& c14,const CRGBW& c15 )
    {
        for( uint8_t i = 0; i < 2; i++) {
            entries[0+i]=c00; entries[2+i]=c01; entries[4+i]=c02; entries[6+i]=c03;
            entries[8+i]=c04; entries[10+i]=c05; entries[12+i]=c06; entries[14+i]=c07;
            entries[16+i]=c08; entries[18+i]=c09; entries[20+i]=c10; entries[22+i]=c11;
            entries[24+i]=c12; entries[26+i]=c13; entries[28+i]=c14; entries[30+i]=c15;
        }
    };
    
    CRGBWPalette32( const CRGBWPalette32& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CRGBWPalette32( const CRGBW rhs[32])
    {
        memmove8( &(entries[0]), &(rhs[0]), sizeof( entries));
    }
    CRGBWPalette32& operator=( const CRGBWPalette32& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }
    CRGBWPalette32& operator=( const CRGBW rhs[32])
    {
        memmove8( &(entries[0]), &(rhs[0]), sizeof( entries));
        return *this;
    }
    
    CRGBWPalette32( const CHSVPalette32& rhs)
    {
        for( uint8_t i = 0; i < 32; i++) {
            entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
        }
    }
    CRGBWPalette32( const CHSV rhs[32])
    {
        for( uint8_t i = 0; i < 32; i++) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
    }
    CRGBWPalette32& operator=( const CHSVPalette32& rhs)
    {
        for( uint8_t i = 0; i < 32; i++) {
            entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
        }
        return *this;
    }
    CRGBWPalette32& operator=( const CHSV rhs[32])
    {
        for( uint8_t i = 0; i < 32; i++) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
        return *this;
    }
    
    CRGBWPalette32( const TProgmemRGBPalette32& rhs)
    {
        for( uint8_t i = 0; i < 32; i++) {
            entries[i] =  FL_PGM_READ_DWORD_NEAR( rhs + i);
        }
    }
    CRGBWPalette32& operator=( const TProgmemRGBPalette32& rhs)
    {
        for( uint8_t i = 0; i < 32; i++) {
            entries[i] =  FL_PGM_READ_DWORD_NEAR( rhs + i);
        }
        return *this;
    }
    
    bool operator==( const CRGBWPalette32 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint8_t i = 0; i < (sizeof( entries)); i++) {
            if( *p != *q) return false;
            p++;
            q++;
        }
        return true;
    }
    bool operator!=( const CRGBWPalette32 rhs)
    {
        return !( *this == rhs);
    }
    
    inline CRGBW& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CRGBW& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }
    
    inline CRGBW& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CRGBW& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    
    operator CRGBW*()
    {
        return &(entries[0]);
    }
    
    CRGBWPalette32( const CHSV& c1)
    {
        fill_solid( &(entries[0]), 32, c1);
    }
    CRGBWPalette32( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 32, c1, c2);
    }
    CRGBWPalette32( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 32, c1, c2, c3);
    }
    CRGBWPalette32( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 32, c1, c2, c3, c4);
    }
    
    CRGBWPalette32( const CRGBW& c1)
    {
        fill_solid( &(entries[0]), 32, c1);
    }
    CRGBWPalette32( const CRGBW& c1, const CRGBW& c2)
    {
        fill_gradient_RGB( &(entries[0]), 32, c1, c2);
    }
    CRGBWPalette32( const CRGBW& c1, const CRGBW& c2, const CRGBW& c3)
    {
        fill_gradient_RGB( &(entries[0]), 32, c1, c2, c3);
    }
    CRGBWPalette32( const CRGBW& c1, const CRGBW& c2, const CRGBW& c3, const CRGBW& c4)
    {
        fill_gradient_RGB( &(entries[0]), 32, c1, c2, c3, c4);
    }
    
    
    CRGBWPalette32( const CRGBWPalette16& rhs16)
    {
        UpscalePalette( rhs16, *this);
    }
    CRGBWPalette32& operator=( const CRGBWPalette16& rhs16)
    {
        UpscalePalette( rhs16, *this);
        return *this;
    }
    
    CRGBWPalette32( const TProgmemRGBPalette16& rhs)
    {
        CRGBWPalette16 p16(rhs);
        *this = p16;
    }
    CRGBWPalette32& operator=( const TProgmemRGBPalette16& rhs)
    {
        CRGBWPalette16 p16(rhs);
        *this = p16;
        return *this;
    }
    
    
    // Gradient palettes are loaded into CRGBW16Palettes in such a way
    // that, if possible, every color represented in the gradient palette
    // is also represented in the CRGBWPalette32.
    // For example, consider a gradient palette that is all black except
    // for a single, one-element-wide (1/256th!) spike of red in the middle:
    //     0,   0,0,0
    //   124,   0,0,0
    //   125, 255,0,0  // one 1/256th-palette-wide red stripe
    //   126,   0,0,0
    //   255,   0,0,0
    // A naive conversion of this 256-element palette to a 16-element palette
    // might accidentally completely eliminate the red spike, rendering the
    // palette completely black.
    // However, the conversions provided here would attempt to include a
    // the red stripe in the output, more-or-less as faithfully as possible.
    // So in this case, the resulting CRGBWPalette32 palette would have a red
    // stripe in the middle which was 1/16th of a palette wide -- the
    // narrowest possible in a CRGBWPalette32.
    // This means that the relative width of stripes in a CRGBWPalette32
    // will be, by definition, different from the widths in the gradient
    // palette.  This code attempts to preserve "all the colors", rather than
    // the exact stripe widths at the expense of dropping some colors.
    CRGBWPalette32( TProgmemRGBGradientPalette_bytes progpal )
    {
        *this = progpal;
    }
    CRGBWPalette32& operator=( TProgmemRGBGradientPalette_bytes progpal )
    {
        TRGBGradientPaletteEntryUnion* progent = (TRGBGradientPaletteEntryUnion*)(progpal);
        TRGBGradientPaletteEntryUnion u;
        
        // Count entries
        uint16_t count = 0;
        do {
            u.dword = FL_PGM_READ_DWORD_NEAR(progent + count);
            count++;;
        } while ( u.index != 255);
        
        int8_t lastSlotUsed = -1;
        
        u.dword = FL_PGM_READ_DWORD_NEAR( progent);
        CRGBW rgbstart( u.r, u.g, u.b, u.w);
        
        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while( indexstart < 255) {
            progent++;
            u.dword = FL_PGM_READ_DWORD_NEAR( progent);
            int indexend  = u.index;
            CRGBW rgbend( u.r, u.g, u.b, u.w);
            istart8 = indexstart / 8;
            iend8   = indexend   / 8;
            if( count < 16) {
                if( (istart8 <= lastSlotUsed) && (lastSlotUsed < 31)) {
                    istart8 = lastSlotUsed + 1;
                    if( iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB( &(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
    CRGBWPalette32& loadDynamicGradientPalette( TDynamicRGBGradientPalette_bytes gpal )
    {
        TRGBGradientPaletteEntryUnion* ent = (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;
        
        // Count entries
        uint16_t count = 0;
        do {
            u = *(ent + count);
            count++;;
        } while ( u.index != 255);
        
        int8_t lastSlotUsed = -1;
        
        
        u = *ent;
        CRGBW rgbstart( u.r, u.g, u.b, u.w);
        
        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while( indexstart < 255) {
            ent++;
            u = *ent;
            int indexend  = u.index;
            CRGBW rgbend( u.r, u.g, u.b, u.w);
            istart8 = indexstart / 8;
            iend8   = indexend   / 8;
            if( count < 16) {
                if( (istart8 <= lastSlotUsed) && (lastSlotUsed < 31)) {
                    istart8 = lastSlotUsed + 1;
                    if( iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB( &(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
    
};



class CRGBWPalette256 {
public:
    CRGBW entries[256];
    CRGBWPalette256() {};
    CRGBWPalette256( const CRGBW& c00,const CRGBW& c01,const CRGBW& c02,const CRGBW& c03,
                  const CRGBW& c04,const CRGBW& c05,const CRGBW& c06,const CRGBW& c07,
                  const CRGBW& c08,const CRGBW& c09,const CRGBW& c10,const CRGBW& c11,
                  const CRGBW& c12,const CRGBW& c13,const CRGBW& c14,const CRGBW& c15 )
    {
        CRGBWPalette16 p16(c00,c01,c02,c03,c04,c05,c06,c07,
                          c08,c09,c10,c11,c12,c13,c14,c15);
        *this = p16;
    };

    CRGBWPalette256( const CRGBWPalette256& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
    }
    CRGBWPalette256( const CRGBW rhs[256])
    {
        memmove8( &(entries[0]), &(rhs[0]), sizeof( entries));
    }
    CRGBWPalette256& operator=( const CRGBWPalette256& rhs)
    {
        memmove8( &(entries[0]), &(rhs.entries[0]), sizeof( entries));
        return *this;
    }
    CRGBWPalette256& operator=( const CRGBW rhs[256])
    {
        memmove8( &(entries[0]), &(rhs[0]), sizeof( entries));
        return *this;
    }

    CRGBWPalette256( const CHSVPalette256& rhs)
    {
    	for( int i = 0; i < 256; i++) {
	    	entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
    	}
    }
    CRGBWPalette256( const CHSV rhs[256])
    {
        for( int i = 0; i < 256; i++) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
    }
    CRGBWPalette256& operator=( const CHSVPalette256& rhs)
    {
    	for( int i = 0; i < 256; i++) {
	    	entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
    	}
        return *this;
    }
    CRGBWPalette256& operator=( const CHSV rhs[256])
    {
        for( int i = 0; i < 256; i++) {
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
        }
        return *this;
    }

    CRGBWPalette256( const CRGBWPalette16& rhs16)
    {
        UpscalePalette( rhs16, *this);
    }
    CRGBWPalette256& operator=( const CRGBWPalette16& rhs16)
    {
        UpscalePalette( rhs16, *this);
        return *this;
    }

    CRGBWPalette256( const TProgmemRGBPalette16& rhs)
    {
        CRGBWPalette16 p16(rhs);
        *this = p16;
    }
    CRGBWPalette256& operator=( const TProgmemRGBPalette16& rhs)
    {
        CRGBWPalette16 p16(rhs);
        *this = p16;
        return *this;
    }

    bool operator==( const CRGBWPalette256 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if( p == q) return true;
        for( uint16_t i = 0; i < (sizeof( entries)); i++) {
            if( *p != *q) return false;
            p++;
            q++;
        }
        return true;
    }
    bool operator!=( const CRGBWPalette256 rhs)
    {
        return !( *this == rhs);
    }
    
    inline CRGBW& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CRGBW& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return entries[x];
    }

    inline CRGBW& operator[] (int x) __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CRGBW& operator[] (int x) const __attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }

    operator CRGBW*()
    {
        return &(entries[0]);
    }

    CRGBWPalette256( const CHSV& c1)
    {
        fill_solid( &(entries[0]), 256, c1);
    }
    CRGBWPalette256( const CHSV& c1, const CHSV& c2)
    {
        fill_gradient( &(entries[0]), 256, c1, c2);
    }
    CRGBWPalette256( const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient( &(entries[0]), 256, c1, c2, c3);
    }
    CRGBWPalette256( const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient( &(entries[0]), 256, c1, c2, c3, c4);
    }

    CRGBWPalette256( const CRGBW& c1)
    {
        fill_solid( &(entries[0]), 256, c1);
    }
    CRGBWPalette256( const CRGBW& c1, const CRGBW& c2)
    {
        fill_gradient_RGB( &(entries[0]), 256, c1, c2);
    }
    CRGBWPalette256( const CRGBW& c1, const CRGBW& c2, const CRGBW& c3)
    {
        fill_gradient_RGB( &(entries[0]), 256, c1, c2, c3);
    }
    CRGBWPalette256( const CRGBW& c1, const CRGBW& c2, const CRGBW& c3, const CRGBW& c4)
    {
        fill_gradient_RGB( &(entries[0]), 256, c1, c2, c3, c4);
    }

    CRGBWPalette256( TProgmemRGBGradientPalette_bytes progpal )
    {
        *this = progpal;
    }
    CRGBWPalette256& operator=( TProgmemRGBGradientPalette_bytes progpal )
    {
        TRGBGradientPaletteEntryUnion* progent = (TRGBGradientPaletteEntryUnion*)(progpal);
        TRGBGradientPaletteEntryUnion u;
        u.dword = FL_PGM_READ_DWORD_NEAR( progent);
        CRGBW rgbstart( u.r, u.g, u.b, u.w);

        int indexstart = 0;
        while( indexstart < 255) {
            progent++;
            u.dword = FL_PGM_READ_DWORD_NEAR( progent);
            int indexend  = u.index;
            CRGBW rgbend( u.r, u.g, u.b, u.w);
            fill_gradient_RGB( &(entries[0]), indexstart, rgbstart, indexend, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
    CRGBWPalette256& loadDynamicGradientPalette( TDynamicRGBGradientPalette_bytes gpal )
    {
        TRGBGradientPaletteEntryUnion* ent = (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;
        u = *ent;
        CRGBW rgbstart( u.r, u.g, u.b, u.w);

        int indexstart = 0;
        while( indexstart < 255) {
            ent++;
            u = *ent;
            int indexend  = u.index;
            CRGBW rgbend( u.r, u.g, u.b, u.w);
            fill_gradient_RGB( &(entries[0]), indexstart, rgbstart, indexend, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
};



typedef enum { NOBLEND=0, LINEARBLEND=1 } TBlendType;

CRGBW ColorFromPalette( const CRGBWPalette16& pal,
                      uint8_t index,
                      uint8_t brightness=255,
                      TBlendType blendType=LINEARBLEND);

CRGBW ColorFromPalette( const TProgmemRGBPalette16& pal,
                       uint8_t index,
                       uint8_t brightness=255,
                       TBlendType blendType=LINEARBLEND);

CRGBW ColorFromPalette( const CRGBWPalette256& pal,
                       uint8_t index,
                       uint8_t brightness=255,
                       TBlendType blendType=NOBLEND );

CHSV ColorFromPalette( const CHSVPalette16& pal,
                       uint8_t index,
                       uint8_t brightness=255,
                       TBlendType blendType=LINEARBLEND);

CHSV ColorFromPalette( const CHSVPalette256& pal,
                       uint8_t index,
                       uint8_t brightness=255,
                       TBlendType blendType=NOBLEND );

CRGBW ColorFromPalette( const CRGBWPalette32& pal,
                      uint8_t index,
                      uint8_t brightness=255,
                      TBlendType blendType=LINEARBLEND);

CRGBW ColorFromPalette( const TProgmemRGBPalette32& pal,
                      uint8_t index,
                      uint8_t brightness=255,
                      TBlendType blendType=LINEARBLEND);

CHSV ColorFromPalette( const CHSVPalette32& pal,
                      uint8_t index,
                      uint8_t brightness=255,
                      TBlendType blendType=LINEARBLEND);


// Fill a range of LEDs with a sequece of entryies from a palette
template <typename PALETTE>
void fill_palette(CRGBW* L, uint16_t N, uint8_t startIndex, uint8_t incIndex,
                  const PALETTE& pal, uint8_t brightness, TBlendType blendType)
{
    uint8_t colorIndex = startIndex;
    for( uint16_t i = 0; i < N; i++) {
        L[i] = ColorFromPalette( pal, colorIndex, brightness, blendType);
        colorIndex += incIndex;
    }
}

  template <typename PALETTE>
void map_data_into_colors_through_palette(
    uint8_t *dataArray, uint16_t dataCount,
    CRGBW* targetColorArray,
    const PALETTE& pal,
    uint8_t brightness=255,
    uint8_t opacity=255,
    TBlendType blendType=LINEARBLEND)
{
  for( uint16_t i = 0; i < dataCount; i++) {
    uint8_t d = dataArray[i];
    CRGBW rgb = ColorFromPalette( pal, d, brightness, blendType);
    if( opacity == 255 ) {
      targetColorArray[i] = rgb;
    } else {
      targetColorArray[i].nscale8( 256 - opacity);
      rgb.nscale8_video( opacity);
      targetColorArray[i] += rgb;
    }
  }
}

// nblendPaletteTowardPalette:
//               Alter one palette by making it slightly more like
//               a 'target palette', used for palette cross-fades.
//
//               It does this by comparing each of the R, G, and B channels
//               of each entry in the current palette to the corresponding
//               entry in the target palette and making small adjustments:
//                 If the Red channel is too low, it will be increased.
//                 If the Red channel is too high, it will be slightly reduced.
//                 ... and likewise for Green and Blue channels.
//
//               Additionally, there are two significant visual improvements
//               to this algorithm implemented here.  First is this:
//                 When increasing a channel, it is stepped up by ONE.
//                 When decreasing a channel, it is stepped down by TWO.
//               Due to the way the eye perceives light, and the way colors
//               are represented in RGB, this produces a more uniform apparent
//               brightness when cross-fading between most palette colors.
//
//               The second visual tweak is limiting the number of changes
//               that will be made to the palette at once.  If all the palette
//               entries are changed at once, it can give a muddled appearance.
//               However, if only a few palette entries are changed at once,
//               you get a visually smoother transition: in the middle of the
//               cross-fade your current palette will actually contain some
//               colors from the old palette, a few blended colors, and some
//               colors from the new palette.
//               The maximum number of possible palette changes per call
//               is 48 (sixteen color entries time three channels each).
//               The default 'maximim number of changes' here is 12, meaning
//               that only approximately a quarter of the palette entries
//               will be changed per call.
void nblendPaletteTowardPalette( CRGBWPalette16& currentPalette,
                                CRGBWPalette16& targetPalette,
                                uint8_t maxChanges=24);




//  You can also define a static RGB palette very compactly in terms of a series
//  of connected color gradients.
//  For example, if you want the first 3/4ths of the palette to be a slow
//  gradient ramping from black to red, and then the remaining 1/4 of the
//  palette to be a quicker ramp to white, you specify just three points: the
//  starting black point (at index 0), the red midpoint (at index 192),
//  and the final white point (at index 255).  It looks like this:
//
//    index:  0                                    192          255
//            |----------r-r-r-rrrrrrrrRrRrRrRrRRRR-|-RRWRWWRWWW-|
//    color: (0,0,0)                           (255,0,0)    (255,255,255)
//
//  Here's how you'd define that gradient palette:
//
//    DEFINE_GRADIENT_PALETTE( black_to_red_to_white_p ) {
//          0,      0,  0,  0,    /* at index 0, black(0,0,0) */
//        192,    255,  0,  0,    /* at index 192, red(255,0,0) */
//        255,    255,255,255    /* at index 255, white(255,255,255) */
//    };
//
//  This format is designed for compact storage.  The example palette here
//  takes up just 12 bytes of PROGMEM (flash) storage, and zero bytes
//  of SRAM when not currently in use.
//
//  To use one of these gradient palettes, simply assign it into a
//  CRGBWPalette16 or a CRGBWPalette256, like this:
//
//    CRGBWPalette16 pal = black_to_red_to_white_p;
//
//  When the assignment is made, the gradients are expanded out into
//  either 16 or 256 palette entries, depending on the kind of palette
//  object they're assigned to.
//
//  IMPORTANT NOTES & CAVEATS:
//
//  - The last 'index' position MUST BE 255!  Failure to end with
//    index 255 will result in program hangs or crashes.
//
//  - At this point, these gradient palette definitions MUST BE
//    stored in PROGMEM on AVR-based Arduinos.  If you use the
//    DEFINE_GRADIENT_PALETTE macro, this is taken care of automatically.
//

#define DEFINE_GRADIENT_PALETTE(X) \
  FL_ALIGN_PROGMEM \
  extern const TProgmemRGBGradientPalette_byte X[] FL_PROGMEM =

#define DECLARE_GRADIENT_PALETTE(X) \
  FL_ALIGN_PROGMEM \
  extern const TProgmemRGBGradientPalette_byte X[] FL_PROGMEM


// Functions to apply gamma adjustments, either:
// - a single gamma adjustment to a single scalar value,
// - a single gamma adjustment to each channel of a CRGBW color, or
// - different gamma adjustments for each channel of a CRFB color.
//
// Note that the gamma is specified as a traditional floating point value
// e.g., "2.5", and as such these functions should not be called in
// your innermost pixel loops, or in animations that are extremely
// low on program storage space.  Nevertheless, if you need these
// functions, here they are.
//
// Furthermore, bear in mind that CRGBW leds have only eight bits
// per channel of color resolution, and that very small, subtle shadings
// may not be visible.
uint8_t applyGamma_video( uint8_t brightness, float gamma);
CRGBW    applyGamma_video( const CRGBW& orig, float gamma);
CRGBW    applyGamma_video( const CRGBW& orig, float gammaR, float gammaG, float gammaB);
// The "n" versions below modify their arguments in-place.
CRGBW&  napplyGamma_video( CRGBW& rgb, float gamma);
CRGBW&  napplyGamma_video( CRGBW& rgb, float gammaR, float gammaG, float gammaB);
void   napplyGamma_video( CRGBW* rgbarray, uint16_t count, float gamma);
void   napplyGamma_video( CRGBW* rgbarray, uint16_t count, float gammaR, float gammaG, float gammaB);


FASTLED_NAMESPACE_END

///@}
#endif

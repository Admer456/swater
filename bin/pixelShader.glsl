
#version 330 core

in vec3 fragmentPosition;
in vec2 fragmentCoord;

uniform sampler2D diffuseMap;
uniform sampler2D paletteMap;
uniform float gTime;

out vec4 outColor;

// Index conversion utilities
int Index_F2I( float index )
{
    return int( index * 255.0 );
}

float Index_I2F( int index )
{
    return index / 255.0;
}

// Coordinate conversion utilities
// Must know texture dimensions beforehand! (todo: change this with a simple uniform)
ivec2 Coord_F2I( vec2 coord )
{
    return ivec2( int(coord.x * 128.0), int(coord.y * 128.0) );
}

vec2 Coord_I2F( ivec2 coord )
{
    return vec2( float(coord.x / 128.0), float(coord.y / 128.0) );
}

// Sample an index from this integer coordinate
int SampleIndex( ivec2 coords )
{
    vec2 fcoords = Coord_I2F( coords );
    return Index_F2I( texture( diffuseMap, fcoords ).r );
}

// Samples a colour from the palette
vec3 SampleColor( int index )
{
    // Multiplying by 255/256 prevents the 'palette overflow' issue
    return texture( paletteMap, vec2( Index_I2F( index ) * (255.0/256.0), 0.5 ) ).rgb;
}

// Final sample
// paletteOffset is used to shift the colour in the palette
vec3 SamplePrimary( ivec2 coords, int paletteOffset )
{
    return SampleColor( SampleIndex( coords ) + paletteOffset );
}

int FixIndex( int index )
{
    index = abs( index % 256 );

    // Index 4 is reserved for underwater fog colour and is sometimes
    // drastically different from the rest of the texture's hue
    if ( index == 4 )
        return 5;

    return index;
}

// Integer modulo of time, I don't have any intuitive explanation for this right now
// numFrames is the number of segments into which we'll split time
// frequency is how 'small' one segment is
// E.g. if you wanna get a scrolling vertical texture, that moves 1 pixel down every 0.1 seconds, and the texture dimension is 128 pixels, you go:
// currentTexCoord + ivec2(0, TimeFraction( gTime, 10.0, 128 ))
// Note: the current design assumes we got a 128x128 texture
int TimeFraction( const float time, const float updateRate, const int numFrames )
{
    return int( time * float(numFrames) / updateRate ) % numFrames;
}

void main()
{
    // We move the wave on an integer grid
    ivec2 currentIntCoord = Coord_F2I( fragmentCoord );
    // timeOffset has a range between 0 and 127, and it updates through time like a sawtooth
    // 128 is currently hardcoded but will be replaced with the texture's width
    int timeOffset = TimeFraction( gTime, 8.0, 128 );

    // Static water
    int mainIndex = SampleIndex( currentIntCoord );

    // Mixing waves
    int primaryIndex = SampleIndex( currentIntCoord + ivec2(timeOffset, 32) );
    int secondaryIndex = SampleIndex( currentIntCoord + ivec2(-48, timeOffset) );
    // Instead of mixing their RGB, we mix the indices
    int avgIndex = FixIndex( (primaryIndex + secondaryIndex) / 2 );

    // Draw the static texture as-is
    outColor.rgb = SamplePrimary( currentIntCoord, 0 );

    // This is the "fake ripple" algorithm
    // Without reverse-engineering GoldSRC's software renderer, I can't do much else here!
    if ( bool(avgIndex & 48) && avgIndex > 20 && avgIndex < 192 )
        outColor.rgb = SampleColor( avgIndex );

    // Uncomment this to debug the mixing indices
    //outColor.rgb = vec3( Index_I2F(avgIndex) );
    
    outColor.a = 1.0;
}

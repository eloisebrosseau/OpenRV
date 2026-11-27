//
// Copyright (C) 2026  Autodesk, Inc. All Rights Reserved.
//
// SPDX-License-Identifier: Apache-2.0
//
#if __VERSION__ >= 150
out vec4 FragColor;
#define FRAGCOLOR FragColor
#define SAMPLE_RECT texture
#else
#extension GL_ARB_texture_rectangle : require
#define FRAGCOLOR gl_FragColor
#define SAMPLE_RECT texture2DRect
#endif

uniform sampler2DRect stroke;
uniform sampler2DRect original;
uniform float opacity;

void main()
{
    float coverage = SAMPLE_RECT(stroke, gl_FragCoord.xy).a * opacity;
    FRAGCOLOR = vec4(SAMPLE_RECT(original, gl_FragCoord.xy).rgb, coverage);
}

#version 120

uniform sampler2DRect tex0;

varying vec2 texCoordVarying;

void main()
{
    // Gaussian filter(Y)
    vec4 color;
    float blurAmnt = 1;
    
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * 4.0));
    color += 2.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * 3.0));
    color += 3.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * 2.0));
    color += 4.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * 1.0));
    
    color += 5.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt));
    
    color += 4.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * -1.0));
    color += 3.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * -2.0));
    color += 2.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * -3.0));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(0.0, blurAmnt * -4.0));
   
    color /= 25.0;
    
    // Set
    gl_FragColor = color;
}
#version 120

uniform sampler2DRect tex0;
uniform sampler2DRect maskTex;
uniform int reverseMask;

varying vec2 texCoordVarying;

void main()
{
    // Gaussian filter(X)
    vec4 color;
    float blurAmnt = 1;
    
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt * -3.0, 0.0));
    color += 2.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt * -2.0, 0.0));
    color += 3.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt * -1.0, 0.0));
    
    color += 4.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt, 0));
    
    color += 3.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt * 1.0, 0.0));
    color += 2.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt * 2.0, 0.0));
    color += 1.0 * texture2DRect(tex0, texCoordVarying + vec2(blurAmnt * 3.0, 0.0));
   
    color /= 16.0;
    
    // Get alpha value
    float mask = texture2DRect(maskTex, texCoordVarying).r;

    // Set
    if (reverseMask > 0) {
        gl_FragColor = vec4(color.rgb, 1.0-mask);
    } else {
        gl_FragColor = vec4(color.rgb, mask);
    }
}
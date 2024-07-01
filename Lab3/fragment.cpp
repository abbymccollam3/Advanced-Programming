#version 120

/*
Author: Abby McCollam
Class: ECE4122 SectionA
Last Date Modified: 12/12/23
Description:

Fragment shader file defining lighting for each object
*/

// Interpolated values from the vertex shaders
varying vec2 UV;
varying vec3 Position_worldspace1;
varying vec3 Position_worldspace2;
varying vec3 Position_worldspace3;
varying vec3 Position_worldspace4;

varying vec3 Normal_cameraspace;
varying vec3 Normal_cameraspace2;
varying vec3 Normal_cameraspace3;
varying vec3 Normal_cameraspace4;

varying vec3 EyeDirection_cameraspace1;
varying vec3 EyeDirection_cameraspace2;
varying vec3 EyeDirection_cameraspace3;
varying vec3 EyeDirection_cameraspace4;

varying vec3 LightDirection_cameraspace1;
varying vec3 LightDirection_cameraspace2;
varying vec3 LightDirection_cameraspace3;
varying vec3 LightDirection_cameraspace4;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform vec3 LightPosition_worldspace1;
uniform vec3 LightPosition_worldspace2;
uniform vec3 LightPosition_worldspace3;
uniform vec3 LightPosition_worldspace4;

uniform float LightPower1;
uniform float LightPower2;
uniform float LightPower3;
uniform float LightPower4;

void main()
{
    // Light emission properties
    // You probably want to put them as uniforms
    vec3 LightColor = vec3(1.0,1.0,1.0);

    // Material properties
    vec3 MaterialDiffuseColor = texture2D(myTextureSampler, UV).rgb;
    vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
    vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

    // Distance to the light for first object
    float distance = length( LightPosition_worldspace1 - Position_worldspace1);
    // Normal of the computed fragment, in camera space
    vec3 n = normalize( Normal_cameraspace);
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize(LightDirection_cameraspace1);
    // Cosine of the angle between the normal and the light direction,
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp( dot( n,l ), 0,1 );

    // Eye vector (towards the camera)
    vec3 E = normalize(EyeDirection_cameraspace1);
    // Direction in which the triangle reflects the light
    vec3 R = reflect(-l,n);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    float cosAlpha = clamp( dot( E,R ), 0,1 );

    vec3 resultingColor =
            // Ambient : simulates indirect lighting
            MaterialAmbientColor +
            // Diffuse : "color" of the object
            MaterialDiffuseColor * LightColor * LightPower1 * cosTheta / (distance*distance) +
            // Specular : reflective highlight, like a mirror
            MaterialSpecularColor * LightColor * LightPower1 * pow(cosAlpha,5) / (distance*distance);

    // Distance to the light for second object
    distance = length(LightPosition_worldspace2 - Position_worldspace2);
    l = normalize(LightDirection_cameraspace2);

    resultingColor +=
            // Ambient : simulates indirect lighting
            MaterialAmbientColor +
            // Diffuse : "color" of the object
            MaterialDiffuseColor * LightColor * LightPower2 * cosTheta / (distance*distance) +
            // Specular : reflective highlight, like a mirror
            MaterialSpecularColor * LightColor * LightPower2 * pow(cosAlpha,5) / (distance*distance);

    // Distance to the light for third object
    distance = length(LightPosition_worldspace3 - Position_worldspace3);
    l = normalize(LightDirection_cameraspace3);

    resultingColor +=
            // Ambient : simulates indirect lighting
            MaterialAmbientColor +
            // Diffuse : "color" of the object
            MaterialDiffuseColor * LightColor * LightPower3 * cosTheta / (distance*distance) +
            // Specular : reflective highlight, like a mirror
            MaterialSpecularColor * LightColor * LightPower3 * pow(cosAlpha,5) / (distance*distance);

    // Distance to the light for fourth object
    distance = length(LightPosition_worldspace4 - Position_worldspace4);
    l = normalize(LightDirection_cameraspace4);

    resultingColor +=
            // Ambient : simulates indirect lighting
            MaterialAmbientColor +
            // Diffuse : "color" of the object
            MaterialDiffuseColor * LightColor * LightPower4 * cosTheta / (distance*distance) +
            // Specular : reflective highlight, like a mirror
            MaterialSpecularColor * LightColor * LightPower4 * pow(cosAlpha,5) / (distance*distance);

    gl_FragColor.rgb = resultingColor;
}

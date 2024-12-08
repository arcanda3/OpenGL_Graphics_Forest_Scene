#version 120

uniform float uTime;           // Control for animation timing
uniform float uTurnIntensity;  // Intensity of the turning animation
uniform float uEnableTurn;     // Toggle turning (0.0 = off, 1.0 = on)
uniform float uTurnDuration;   // Time for one turning motion
uniform float uPauseDuration;  // Time for the pause between turns

varying vec2 vST;              // Texture coordinates
varying vec3 vN;               // Normal vector
varying vec3 vL;               // Vector to light
varying vec3 vE;               // Vector to eye

const vec3 LIGHTPOS = vec3(20.0, 60.0, 25.0); // Position of the light source

void main() {
    vST = gl_MultiTexCoord0.st;
    vec3 vert = gl_Vertex.xyz; 

    // Apply turning animation if enabled
    if (uEnableTurn > 0.0) {
        // Calculate the total cycle time
        float uTotalCycleTime = uTurnDuration + uPauseDuration;

        // Determine the current phase in the cycle
        float phaseTime = mod(uTime, uTotalCycleTime);

        if (phaseTime < uTurnDuration) {
            float cyclePhase = phaseTime / uTurnDuration;
            float turnFactor = sin(cyclePhase * 3.14159 * 2.0); // Sinusoidal oscillation

            if (vert.y < 2.0) { // Apply turning to the front side (starting at neck)
                turnFactor *= uTurnIntensity;

                // Parabolic influence based on vertex y-position
                turnFactor *= (vert.y * vert.y);

                // Apply turning to the x-axis
                vert.x += turnFactor;
            }
        }
    }

    vec4 ECposition = gl_ModelViewMatrix * vec4(vert, 1.0);
    vN = normalize(gl_NormalMatrix * gl_Normal);
    vL = LIGHTPOS - ECposition.xyz;
    vE = vec3(0.0, 0.0, 0.0) - ECposition.xyz;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(vert, 1.0);
}

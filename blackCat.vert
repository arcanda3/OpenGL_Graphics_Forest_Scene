#version 120

uniform float uTime;           // Control for animation timing
uniform float uTurnIntensity;  // Intensity of the turning animation
uniform float uRunCycleTime;   // Time for one running cycle (from narrow to wide and back)
uniform float uMaxBend;        // Maximum amount of bend intensity for running
uniform float uEnableTurn;     // Toggle turning animation (0.0 = off, 1.0 = on)
uniform float uEnableRun;      // Toggle running animation (0.0 = off, 1.0 = on)
uniform float uTurnDuration;   // Time for one turning motion
uniform float uPauseDuration;  // Time for pause between turns

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
        // Calculate total cycle time
        float totalCycleTime = uTurnDuration + uPauseDuration;

        // Determine the current phase in the cycle
        float phaseTime = mod(uTime, totalCycleTime);

        if (phaseTime < uTurnDuration) {
            float cyclePhase = phaseTime / uTurnDuration;                
            float turnFactor = sin(cyclePhase * 3.14159 * 2.0); // Sinusoidal oscillation

            // Symmetric motion adjustment
            turnFactor = abs(turnFactor) * 2.0 - 1.0;

            if (vert.y < 0.0) { // Apply turning to the front side
                turnFactor *= uTurnIntensity;

                // Parabolic influence based on vertex y-position
                turnFactor *= (vert.y * vert.y);

                // Apply turning to the x-axis 
                vert.x += turnFactor;
            }
        }
    }

    // Apply running animation if enabled
    if (uEnableRun > 0.0) {
        float totalCycleTime = uRunCycleTime;
        float cyclePhase = mod(uTime, totalCycleTime) / totalCycleTime; // Normalize time for the cycle

        // Use a smooth sine wave for smooth bending and stretching
        float bendFactor = sin(cyclePhase * 3.14159 * 2.0);
        bendFactor *= uMaxBend; // Control the maximum bend intensity
        float stretchFactor = 1.5; // Stretch factor when bending reaches the negative peak

        // Apply the bending/stretching effect to the z-axis
        if (bendFactor > 0.0) {
            // Bend forward (narrow)
            vert.z += -(bendFactor * (vert.y * vert.y));  
        } else {
            // Stretch backward (wide)
            vert.z += (bendFactor * (vert.y * vert.y) * stretchFactor);  // Increase stretch amount
        }
    }

    vec4 ECposition = gl_ModelViewMatrix * vec4(vert, 1.0);
    vN = normalize(gl_NormalMatrix * gl_Normal);
    vL = LIGHTPOS - ECposition.xyz;
    vE = vec3(0.0, 0.0, 0.0) - ECposition.xyz;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(vert, 1.0);
}

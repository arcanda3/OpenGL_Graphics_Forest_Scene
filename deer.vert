#version 120

uniform float uTime;                // Control for animation timing
uniform float uTurnIntensity;       // Intensity of the turning animation
uniform float uEnableTurn;          // Toggle turning (0.0 = off, 1.0 = on)
uniform float uGrazingCycleTime;    // Time for one grazing cycle
uniform float uTurnDuration;        // Time for one turning motion
uniform float uPauseDuration;       // Time for the pause between turns
uniform float uEnableGrazing;       // Toggle grazing animation (0.0 = off, 1.0 = on)
uniform float uGrazingIntensity;    // Intensity of grazing motion

varying vec2 vST;                   // Texture coordinates
varying vec3 vN;                    // Normal vector
varying vec3 vL;                    // Vector to light
varying vec3 vE;                    // Vector to eye

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

            if (vert.x > 0.0) { // Apply turning to the front side
                turnFactor *= uTurnIntensity; 

                // Parabolic influence based on vertex x-position
                turnFactor *= (vert.x * vert.x);

                // Apply the turn to the y-axis
                vert.y += turnFactor;
            }
        }
    }

	// Apply grazing animation if enabled
    if (uEnableGrazing > 0.0) {
        // Calculate the grazing cycle phase
        float cyclePhase = mod(uTime, uGrazingCycleTime) / uGrazingCycleTime;
    	float bendFactor = abs(sin(cyclePhase * 3.14159));

		if (vert.x > 1.0) { // Apply grazing to the front side (starting at neck)
			float inverseFactor = 1.0 / (vert.x + 0.1);
			bendFactor *= inverseFactor * uGrazingIntensity;

            // Apply parabolic bend along y-axis
			vert.z += -(bendFactor * (vert.x * vert.x));
		}
    }

    vec4 ECposition = gl_ModelViewMatrix * vec4(vert, 1.0);
    vN = normalize(gl_NormalMatrix * gl_Normal);
    vL = LIGHTPOS - ECposition.xyz;
    vE = vec3(0.0, 0.0, 0.0) - ECposition.xyz;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(vert, 1.0);
}

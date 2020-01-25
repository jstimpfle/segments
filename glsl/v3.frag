#version 130

uniform mat4 screenTransform;

in vec3 positionF;
in vec3 normalF;
in vec3 colorF;

float compute_specular_strength(vec3 lightPos, vec3 surfacePoint, vec3 normalizedSurfaceNormal, vec3 spectatorPosition) {
	vec3 lightToSurface = surfacePoint - lightPos;
	vec3 reflectVector = normalize(lightToSurface - 2.0 * dot(lightToSurface, normalizedSurfaceNormal) * normalizedSurfaceNormal);
	vec3 spectateDirection = normalize(spectatorPosition - surfacePoint);
	float strength = pow(clamp(dot(reflectVector, spectateDirection), 0, 1), 2.0);
	return strength;
}

float compute_diffuse_strength(vec3 lightPos, vec3 surfacePoint, vec3 normalizedSurfaceNormal)
{
	vec3 lightToSurface = normalize(surfacePoint - lightPos);
	return clamp(dot(lightToSurface, normalizedSurfaceNormal), 0, 1);
}


void main()
{
	vec3 spec = vec3(0,0,-3.5);
	vec3 light = (vec4(5,5,-3.5,1) * screenTransform).xyz;
	vec3 normal = normalize(normalF);
	
	float diffuseStrength = compute_diffuse_strength(light, positionF, normal);  //compute_specular_strength(light, positionF, normalF, spec);
	diffuseStrength = clamp(diffuseStrength, 0, 1);
	float lightIntensity = 0.5 + diffuseStrength;
	if (lightIntensity < 0) lightIntensity = 0;
	if (lightIntensity > 1) lightIntensity = 1;
	gl_FragColor = vec4(lightIntensity * colorF, 1);
	//gl_FragColor = vec4(positionF, 1);
}

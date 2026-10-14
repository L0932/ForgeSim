#version 460 core

const float pi = 3.14159265359;
const float sampleDelta = 0.05;

in vec3 vLocalPosition;

uniform samplerCube uEnvironmentMap;

out vec4 fragColor;

void main()
{
	vec3 normal =
		normalize(vLocalPosition);

	vec3 referenceAxis =
		abs(normal.y) < 0.999
		? vec3(0.0, 1.0, 0.0)
		: vec3(0.0, 0.0, 1.0);

	vec3 tangent =
		normalize(
			cross(
				referenceAxis,
				normal));

	vec3 bitangent =
		cross(
			normal,
			tangent);

	vec3 irradiance = vec3(0.0);
	float sampleCount = 0.0;

	for (float phi = 0.0;
		phi < 2.0 * pi;
		phi += sampleDelta)
	{
		for (float theta = 0.0;
			theta < 0.5 * pi;
			theta += sampleDelta)
		{
			float sineTheta = sin(theta);
			float cosineTheta = cos(theta);

			vec3 tangentDirection =
				vec3(
					sineTheta * cos(phi),
					sineTheta * sin(phi),
					cosineTheta);

			vec3 sampleDirection =
				tangent *
					tangentDirection.x +
				bitangent *
					tangentDirection.y +
				normal *
					tangentDirection.z;

			irradiance +=
				texture(
					uEnvironmentMap,
					sampleDirection).rgb *
				cosineTheta *
				sineTheta;

			sampleCount += 1.0;
		}
	}

	irradiance *=
		pi /
		max(sampleCount, 1.0);

	fragColor = vec4(
		irradiance,
		1.0);
}
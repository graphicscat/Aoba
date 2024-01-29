#define PI 3.141592654

float mgroundRadiusMM = 6.360;
float matmosphereRadiusMM = 6.460;

vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 RGBConvert(vec3 c)
{

  return vec3(c.x/255.0,c.y/255.0,c.z/255.0);

}

float safeAcos(float x)
{
    return acos(clamp(x,-1.0,1.0));
}

float rayIntersectSphere(vec3 ro, vec3 rd, float rad) {
    float b = dot(ro, rd);
    float c = dot(ro, ro) - rad*rad;
    if (c > 0.0f && b > 0.0) return -1.0;
    float discr = b*b - c;
    if (discr < 0.0) return -1.0;
    // Special case: inside sphere, use far discriminant
    if (discr > b*b) return (-b + sqrt(discr));
    return -b - sqrt(discr);
}

float getRayleighPhase(float cosTheta) {
    const float k = 3.0/(16.0*PI);
    return k*(1.0+cosTheta*cosTheta);
}



vec3 viewPos = vec3(0.0, mgroundRadiusMM + 0.0002, 0.0);

vec3 getValFromTLUT(sampler2D tex, vec3 pos, vec3 sunDir) {

    vec2 mtransSkyLUTRes = vec2(256.0,144.0);
    vec2 tLUTRes = mtransSkyLUTRes.xy; 
    float height = length(pos);
    vec3 up = pos / height;
	  float sunCosZenithAngle = dot(sunDir, up);
    vec2 uv = vec2(clamp(0.5 + 0.5*sunCosZenithAngle, 0.0, 1.0),
                   max(0.0, min(1.0, (height - mgroundRadiusMM)/(matmosphereRadiusMM - mgroundRadiusMM))));
    uv.y = 1.0 - uv.y;
    return texture(tex, uv).rgb;
}

void getScatteringValues(vec3 pos, 
                         out vec3 rayleighScattering, 
                         out float mieScattering,
                         out vec3 extinction) {

    vec3 mrayleighScatteringBase = vec3(5.802, 13.558, 33.1);

    float altitudeKM = (length(pos)-mgroundRadiusMM)*1000.0;
    // Note: Paper gets these switched up.

    float rayleighDensity = exp(-altitudeKM/8.0);
    //float mieDensity = exp(-altitudeKM/1.2);
    
    rayleighScattering = mrayleighScatteringBase*rayleighDensity;

    float rayleighAbsorptionBase = 0.0;

    float rayleighAbsorption = rayleighAbsorptionBase*rayleighDensity;
    
    // mieScattering = mieScatteringBase*mieDensity;
    // float mieAbsorption = mieAbsorptionBase*mieDensity;
    
    //vec3 ozoneAbsorption = ozoneAbsorptionBase*max(0.0, 1.0 - abs(altitudeKM-25.0)/15.0);
    
    //extinction = rayleighScattering + rayleighAbsorption + mieScattering + mieAbsorption + ozoneAbsorption;

    extinction = rayleighScattering ;
}


float getSunAltitude(float time)
{
    const float periodSec = 120.0;
    const float halfPeriod = periodSec / 2.0;
    const float sunriseShift = 0.1;
    float cyclePoint = (1.0 - abs((mod(time,periodSec)-halfPeriod)/halfPeriod));
    cyclePoint = (cyclePoint*(1.0+sunriseShift))-sunriseShift;
    return (0.5*PI)*cyclePoint;
}
vec3 getSunDir(float time)
{
    float altitude = getSunAltitude(time);
    return normalize(vec3(0.0, sin(altitude), -cos(altitude)));
}
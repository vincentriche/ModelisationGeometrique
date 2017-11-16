const int Steps = 1000;
const float Epsilon = 0.05; // Marching epsilon
const float T = 0.5;

const float rA = 10.0; // Maximum ray marching or sphere tracing distance from origin
const float rB = 50.0; // Minimum

const float OverRelaxationFactor = 1.8;
const float LipschitzConstant = 2.0;

// Colors
const vec3 body = vec3(250.0 / 255.0, 180.0 / 255.0, 100.0 / 255.0);
const vec3 hair = vec3(180.0 / 255.0, 130.0 / 255.0, 40.0 / 255.0);
const vec3 tshirt = vec3(80.0 / 255.0, 40.0 / 255.0, 100.0 / 255.0);
const vec3 hands = vec3(230.0 / 255.0, 150.0 / 255.0, 70.0 / 255.0);
const vec3 shoes = vec3(110.0 / 255.0, 100.0 / 255.0, 110.0 / 255.0);
const vec3 shoes2 = vec3(200.0 / 255.0, 210.0 / 255.0, 100.0 / 255.0);
const vec3 scarf = vec3(200.0 / 255.0, 20.0 / 255.0, 20.0 / 255.0);
const vec3 eyes = vec3(0.9, 0.9, 0.9);
const vec3 eyes2 = vec3(0.0, 0.0, 0.0);
const vec3 firefly = vec3(75.0 / 255.0, 280.0 / 255.0, 220.0 / 255.0);
const vec3 firefly2 = vec3(200.0 / 255.0, 210.0 / 255.0, 100.0 / 255.0);
const vec3 box = vec3(80.0 / 255.0, 50.0 / 255.0, 25.0 / 255.0);
const vec3 flower1 = vec3(0.0 / 255.0, 0.0 / 255.0, 128.0 / 255.0);
const vec3 flower2 = vec3(128.0 / 255.0, 128.0 / 255.0, 0.0 / 255.0);
const vec3 flower3 = vec3(128.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0);
const vec3 flower4 = vec3(0.0 / 255.0, 128.0 / 255.0, 0.0 / 255.0);
const vec3 ground = vec3(115.0 / 255.0, 110.0 / 255.0, 100.0 / 255.0);

// Transforms
vec3 rotateX(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(p.x, ca * p.y - sa * p.z, sa * p.y + ca * p.z);
}

vec3 rotateY(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(ca * p.x + sa * p.z, p.y, -sa * p.x + ca * p.z);
}

vec3 rotateZ(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(ca * p.x + sa * p.y, -sa * p.x + ca * p.y, p.z);
}

vec3 backAndForthLine(vec3 a, vec3 b, float s)
{
    float t = cos(iTime * s) * 0.5 + 0.5;
    return (1.0 - t) * a + b * t;
}

float scaleAnim(float a, float b, float s)
{
    float t = cos(iTime * s) * 0.5 + 0.5;
    return (1.0 - t) * a + b * t;
}

vec3 rotateXAround(vec3 c, float r, float s)
{
    float t = iTime;
    return vec3(c.x + sin(t * s) * r, c.y, c.z + cos(t * s) * r);
}

vec3 rotateZAround(vec3 c, float r, float s)
{
    float t = iTime;
    return vec3(c.x, c.y + sin(t * s) * r, c.z + cos(t * s) * r);
}

/*
    Smooth falloff function
    r : small radius
    R : Large radius
*/
float falloff(float r, float R)
{
    float x = clamp(r / R, 0.0, 1.0);
    float y = (1.0 - x * x);
    return y * y * y;
}

// Operators
float Blend(float a, float b)
{
    return a + b;
}

float Union(float a, float b)
{
    return max(a, b);
}

float Inter(float a, float b)
{
    return min(a, b);
}

float Minus(float a, float b)
{
    return min(a, 2.0 * T - b);
}

float Substract(float a, float b)
{
    return a - b;
}

float Neg(float a)
{
    return 2.0 * T - a;
}

float BlendN(float a, float b, float n)
{
    return pow(pow(a, n) + pow(b, n), 1.0 / n);
}

// Primitive functions
/* 
    Point skeleton
    p : point
    c : center of skeleton
    e : energy associated to skeleton
    R : large radius
*/
float point(vec3 p, vec3 c, float e, float R)
{
    return e * falloff(length(p - c), R);
}

/*
    Segment skeleton
    p : point
    a : begin point
    b : end point
    e : energy associated to skeleton
    R : large radius
*/
float segment(vec3 p, vec3 a, vec3 b, float e, float R)
{
    float d = dot(p - a, b - a);
    float l = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y) + (b.z - a.z) * (b.z - a.z);
    float v = d / l;

    vec3 X;
    if (v < 0.0)
    {
        X.x = a.x;
        X.y = a.y;
        X.z = a.z;
    }
    else if (v > 1.0)
    {
        X.x = b.x;
        X.y = b.y;
        X.z = b.z;
    }
    else
    {
        X.x = a.x + v * (b.x - a.x);
        X.y = a.y + v * (b.y - a.y);
        X.z = a.z + v * (b.z - a.z);
    }
    float dist = sqrt(pow((p.x - X.x), 2.0) + pow((p.y - X.y), 2.0) + pow((p.z - X.z), 2.0));
    return e * falloff(dist, R);
}

/*
    Plane skeleton
    p : point
    c : center of skeleton
    n : normal point
    e : energy associated to skeleton
    R : large radius
*/
float plane(vec3 p, vec3 c, vec3 n, float e, float R)
{
    float h = -dot(p - c, n) / dot(n, n);
    float d = length(p - (p + h * n));
    return e * falloff(d, R);
}

/*
    Circle skeleton
    p : point
    c : center of skeleton
    n : normal point
    e : energy associated to skeleton
    R : large radius
*/
float circle(vec3 p, vec3 c, vec3 n, float e, float R)
{
    float h = dot(p - c, n);
    float l = sqrt(pow(length(p - c), 2.0) - pow(h, 2.0));
    float m = l - R;
    float d = sqrt(pow(m, 2.0) + pow(h, 2.0));
    return e * falloff(d, R);
}

/*
    Disk skeleton
    p : point
    c : center of skeleton
    n : normal point
    e : energy associated to skeleton
    R : large radius
*/
float disk(vec3 p, vec3 c, vec3 n, float e, float R)
{
    float h = dot(p - c, n);
    float l = sqrt(pow(length(p - c), 2.0) - pow(h, 2.0));
    if (l < R)
        return e * falloff(abs(h), R);
    float m = l - R;
    float d = sqrt(pow(m, 2.0) + pow(h, 2.0));
    return e * falloff(d, R);
}

float cube(vec3 p, vec3 c, float e, float R)
{
    float d = 0.0;
    float w = R / 2.0f;
    vec3 pmin = vec3(c.x - w, c.y - w, c.z - w);
    vec3 pmax = vec3(c.x + w, c.y + w, c.z + w);

    if (p.x < pmin.x)
        d += (p.x - pmin.x) * (p.x - pmin.x);
    else if (p.x > pmax.x)
        d += (p.x - pmax.x) * (p.x - pmax.x);

    if (p.y < pmin.y)
        d += (p.y - pmin.y) * (p.y - pmin.y);
    else if (p.y > pmax.y)
        d += (p.y - pmax.y) * (p.y - pmax.y);

    if (p.z < pmin.z)
        d += (p.z - pmin.z) * (p.z - pmin.z);
    else if (p.z > pmax.z)
        d += (p.z - pmax.z) * (p.z - pmax.z);

    return e * falloff(sqrt(d), R);
}

/*
    Potential field of the object
    p : point
    c : out color
*/
float object(vec3 p, out vec3 c)
{
    p.z = -p.z;

    // Animations
    vec3 anim1 = backAndForthLine(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.3, 0.0), 1.0);
    vec3 anim2 = backAndForthLine(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.4, 0.0), 1.0);
    vec3 anim3 = backAndForthLine(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.4, 0.0), 5.0);

    vec3 anim4 = rotateXAround(vec3(0.0, 5.0 + anim3.y, 0.0), 7.0, 0.3);
    vec3 anim5 = rotateXAround(vec3(2.0, 6.0 + anim3.y, 2.0), 7.0, 0.3);
    float anim6 = scaleAnim(1.5, 2.0, 0.8);
    float anim7 = scaleAnim(1.0, 1.5, 0.5);

    // Objects
    float s1 = plane(p, vec3(0.0, -4.0, 0.0), vec3(0.0, -1.0, 0.0), 1.0, 1.0);

    // Feet
    float s2 = segment(p, vec3(-2.0, -3.0, 0.0), vec3(-2.0, -3.0, -1.25f), 1.0, 2.0);
    float s3 = point(p, vec3(-2.0, -2.75f, -1.0), 1.0, 1.5);
    float s4 = segment(p, vec3(2.0, -3.0, 0.0), vec3(2.0, -3.0, -1.25f), 1.0, 2.0);
    float s5 = point(p, vec3(2.0, -2.75f, -1.0), 1.0, 1.5);

    // Trunk
    float s6 = segment(p, vec3(0.0, 0.0 + anim1.y, 0.25f), vec3(0.0, 2.0 + anim1.y, 0.0), 1.0, 3.0);
    float s7 = segment(p, vec3(-1.5, 2.5 + anim1.y, 0.0), vec3(-3.0, -1.0 + anim1.y, 1.0), 1.0, 1.5);
    float s8 = segment(p, vec3(1.5, 2.5 + anim1.y, 0.0), vec3(3.0, -1.0 + anim1.y, 1.0), 1.0, 1.5);
    float s9 = disk(p, vec3(0.0, -1.1 + anim1.y, 0.0), vec3(0.0, 1.0, 0.0), 1.0, 1.0);
    float s10 = disk(p, vec3(0.0, 3.4 + anim1.y, 0.0), vec3(0.0, 1.0, 0.0), 1.0, 0.7);

    // Head
    float s11 = segment(p, vec3(0.0, 5.5 + anim1.y, 0.0), vec3(0.0, 8.0 + anim1.y, 0.0), 1.0, 2.0);
    float s12 = segment(p, vec3(0.0, 5.5 + anim1.y, 0.0), vec3(0.0, 5.5 + anim1.y, -1.5), 1.0, 1.0);
    float s13 = point(p, vec3(-0.65f, 5.5 + anim1.y, -1.5), 1.0, 2.25f);
    float s14 = point(p, vec3(0.65f, 5.5 + anim1.y, -1.5), 1.0, 2.25f);
    float s15 = point(p, vec3(-0.65f, 7.25f + anim1.y, -0.75f), 1.0, 1.5);
    float s16 = point(p, vec3(0.65f, 7.25f + anim1.y, -0.75f), 1.0, 1.5);
    float s17 = point(p, vec3(-0.4, 7.2 + anim1.y, -1.4), 1.0, 0.25f);
    float s18 = point(p, vec3(0.4, 7.2 + anim1.y, -1.4), 1.0, 0.25f);

    // Hair
    float s19 = segment(p, vec3(-0.25f, 8.5 + anim2.y, 0.0), vec3(1.25f, 9.25f + anim2.y, -2.25f), 1.0, 0.7);
    float s20 = segment(p, vec3(0.25f, 8.5 + anim2.y, 0.0), vec3(2.0, 9.25f + anim2.y, -2.25f), 1.0, 0.7);
    float s21 = segment(p, vec3(-0.25f, 8.5 + anim2.y, 0.0), vec3(-2.0, 9.25f + anim2.y, -2.25f), 1.0, 0.7);
    float s22 = segment(p, vec3(0.25f, 8.5 + anim2.y, 0.0), vec3(-3.0, 9.25f + anim2.y, -2.25f), 1.0, 0.7);

    // Arms
    float s23 = point(p, vec3(-3.8, 1.8 + anim2.y, 0.0), 1.0, 1.5);
    float s24 = segment(p, vec3(-3.8, 1.8 + anim2.y, 0.0), vec3(-2.8, 2.2 + anim2.y, -0.8), 1.0, 0.5);
    float s25 = point(p, vec3(3.8, 1.8 + anim2.y, 0.0), 1.0, 1.5);
    float s26 = segment(p, vec3(3.8, 1.8 + anim2.y, 0.0), vec3(2.8, 2.2 + anim2.y, -0.8), 1.0, 0.5);

    // Fireflies
    float s27 = point(p, anim4, 1.0, anim6);
    float s28 = disk(p, vec3(-0.5 + anim4.x, 0.5 + anim4.y, anim4.z), vec3(2.0, 2.0, 0.0), 1.0, anim6 - 1.3);
    float s29 = disk(p, vec3(0.5 + anim4.x, 0.5 + anim4.y, anim4.z), vec3(2.0, -2.0, 0.0), 1.0, anim6 - 1.3);

    float s30 = point(p, anim5, 1.0, anim7);
    float s31 = disk(p, vec3(-0.5 + anim5.x, 0.5 + anim5.y, anim5.z), vec3(2.0, 2.0, 0.0), 1.0, anim7 - 0.9);
    float s32 = disk(p, vec3(0.5 + anim5.x, 0.5 + anim5.y, anim5.z), vec3(2.0, -2.0, 0.0), 1.0, anim7 - 0.9);

    // Accessories
    float s33 = circle(p, vec3(0.0, 2.0 + anim1.y, -1.25f), vec3(0.0, 0.0, 1.0), 1.0, 0.5);
    float s34 = cube(p, vec3(8.0, -2.0, 0.0), 1.0, 2.0);
    float s35 = cube(p, vec3(9.0, 1.4, 1.0), 1.0, 2.0);

    // Flower 1
    float s36 = segment(p, vec3(-8.0, -4.0, -1.0), vec3(-8.0, -1.8, -1.0), 1.0, 0.3);
    float s37 = disk(p, vec3(-8.0, -2.0, -1.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);
    float s38 = disk(p, vec3(-8.0, -1.0, -1.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);
    float s39 = disk(p, vec3(-7.5, -1.5, -1.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);
    float s40 = disk(p, vec3(-8.5, -1.5, -1.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);

    // Flower 2
    float s41 = segment(p, vec3(-10.0, -4.0, -3.0), vec3(-10.0, -1.8, -3.0), 1.0, 0.3);
    float s42 = disk(p, vec3(-10.0, -2.0, -3.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);
    float s43 = disk(p, vec3(-10.0, -1.0, -3.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);
    float s44 = disk(p, vec3(-9.5, -1.5, -3.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);
    float s45 = disk(p, vec3(-10.5, -1.5, -3.0), normalize(vec3(0.0, 0.0, -1.0)), 1.0, 0.3);

    float v = s1;
    v = BlendN(v, s2, 10.0);
    v = BlendN(v, s3, 10.0);
    v = BlendN(v, s4, 10.0);
    v = BlendN(v, s5, 10.0);
    v = Blend(v, s6);
    v = Substract(v, s7);
    v = Substract(v, s8);
    v = Minus(v, s9);
    v = BlendN(v, s10, 5.0);
    v = Blend(v, s11);
    v = Blend(v, s12);
    v = Blend(v, s13);
    v = Blend(v, s14);
    v = BlendN(v, s15, 5.0);
    v = BlendN(v, s16, 5.0);
    v = Blend(v, s17);
    v = Blend(v, s18);
    v = Blend(v, s19);
    v = Blend(v, s20);
    v = Blend(v, s21);
    v = Blend(v, s22);
    v = Blend(v, s23);
    v = Blend(v, s24);
    v = Blend(v, s25);
    v = Blend(v, s26);
    v = Blend(v, s27);
    v = BlendN(v, s28, 5.0);
    v = BlendN(v, s29, 5.0);
    v = Blend(v, s30);
    v = BlendN(v, s31, 5.0);
    v = BlendN(v, s32, 5.0);
    v = BlendN(v, s33, 10.0);
    v = Union(v, s34);
    v = Union(v, s35);
    v = Union(v, s36);
    v = Union(v, s37);
    v = Union(v, s38);
    v = Union(v, s39);
    v = Union(v, s40);
    v = Union(v, s41);
    v = Union(v, s42);
    v = Union(v, s43);
    v = Union(v, s44);
    v = Union(v, s45);

    c = s1 * ground + s2 * shoes + s3 * shoes2 + s4 * shoes + s5 * shoes2 + s6 * tshirt +
        s7 * tshirt + s8 * tshirt + s9 * tshirt + s10 * scarf + s11 * body + s12 * body + s13 * body +
        s14 * body + s15 * eyes + s16 * eyes + s17 * eyes2 + s18 * eyes2 + s19 * hair + s20 * hair + s21 * hair +
        s22 * hair + s23 * hands + s24 * hands + s25 * hands + s26 * hands + s27 * firefly + s28 * firefly +
        s29 * firefly + s30 * firefly2 + s31 * firefly2 + s32 * firefly2 + s33 * eyes + s34 * box +
        s35 * box + s36 * flower4 + s37 * flower1 + s38 * flower2 + s39 * flower3 + s40 * flower4 +
        s41 * flower4 + s42 * flower1 + s43 * flower2 + s44 * flower3 + s45 * flower4;

    c /= s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15 +
         s16 + s17 + s18 + s19 + s20 + s21 + s22 + s23 + s24 + s25 + s26 + s27 + s28 +
         s29 + s30 + s31 + s32 + s33 + s34 + s35 + s36 + s37 + s38 + s39 + s40 + s41 +
         s42 + s43 + s44 + s45;
    return v - T;
}

/*
    Calculate object normal
    p : point
*/
vec3 ObjectNormal(in vec3 p)
{
    float eps = 0.0001;
    vec3 n;
    vec3 c;
    float v = object(p, c);
    n.x = object(vec3(p.x + eps, p.y, p.z), c) - v;
    n.y = object(vec3(p.x, p.y + eps, p.z), c) - v;
    n.z = object(vec3(p.x, p.y, p.z + eps), c) - v;
    return normalize(n);
}

/*
    Trace ray using ray marching
    o : ray origin
    u : ray direction
    h : hit
    s : Number of steps
    col : out color
*/
float SphereTrace(vec3 o, vec3 u, out bool h, out int s, out vec3 col)
{
    h = false;

    // Don't start at the origin, instead move a little bit forward
    float t = rA;
    vec3 c;
    float step = 0.0;
    float previousRadius = 0.0;
    for (int i = 0; i < Steps; i++)
    {
        s = i;
        vec3 p = o + t * u;
        float v = object(p, c);

        // Enhanced Sphere Tracing
        // Error correction :
        // If we do not intersect the previous sphere
        // Go back. Otherwise move forward.
        float radius = max(Epsilon, abs(v) / LipschitzConstant);
        if (radius + previousRadius < step)
            step -= OverRelaxationFactor * step;
        else
            step = radius * OverRelaxationFactor;
        previousRadius = radius;

        // Hit object
        if (v > 0.0)
        {
            s = i;
            h = true;
            break;
        }
        // Move along ray
        t += step;

        // Escape marched far away
        if (t > rB)
        {
            break;
        }
    }
    col = c;
    return t;
}

// Background color
vec3 background(vec3 rd)
{
    return mix(vec3(0.4, 0.3, 0.0), vec3(0.7, 0.8, 1.0), rd.y * 0.5 + 0.5);
}

/*
     Shading and lighting
    p : point,
    n : normal at point
    color : out color
*/
vec3 Shade(vec3 p, vec3 n, vec3 color)
{
    // point light
    const vec3 lightPos = vec3(5.0, 5.0, 5.0);
    const vec3 lightColor = vec3(0.5, 0.5, 0.5);

    vec3 c = color; //0.25 * background(n);
    vec3 l = normalize(lightPos - p);

    // Not even Phong shading, use weighted cosine instead for smooth transitions
    float diff = 0.5 * (1.0 + dot(n, l));

    c += diff * lightColor;

    return c;
}

// Shading with number of steps
vec3 ShadeSteps(int n)
{
    float t = float(n) / (float(Steps - 1));
    return vec3(t, 0.25 + 0.75 * t, 0.5 - 0.5 * t);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 pixel = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;

    // compute ray origin and direction
    float asp = iResolution.x / iResolution.y;
    vec3 rd = normalize(vec3(asp * pixel.x, pixel.y, -4.0));
    vec3 ro = vec3(0.0, 4.0, 40.0);

    float a = iTime * 0.15;
    //ro = rotateY(ro, a);
    //rd = rotateY(rd, a);

    // Trace ray
    bool hit;

    // Number of steps
    int s;

    vec3 col;
    float t = SphereTrace(ro, rd, hit, s, col);
    vec3 pos = ro + t * rd;

    // Shade background
    vec3 rgb = background(rd);
    if (hit)
    {
        // Compute normal
        vec3 n = ObjectNormal(pos);

        // Shade object with light
        rgb = Shade(pos, n, col);
    }

    // Uncomment this line to shade image with false colors representing the number of steps
    //rgb = ShadeSteps(s);

    fragColor = vec4(rgb, 1.0);
}
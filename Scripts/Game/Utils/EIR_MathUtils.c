// Math utility methods that complement the built-in Math class.
// Does not duplicate: Math.Clamp, Math.Lerp, Math.Map, Math.InverseLerp,
// Math.RandomFloat, Math.RandomInt, Math.Round, Math.Abs, etc.

class EIR_MathUtils
{
	//------------------------------------------------------------------------------------------------
	// Returns -1 if value < 0, 0 if value == 0, 1 if value > 0.
	static int Sign(float value)
	{
		if (value > 0)
			return 1;

		if (value < 0)
			return -1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if a and b are within epsilon of each other.
	// Use instead of == for float comparisons.
	static bool ApproximatelyEqual(float a, float b, float epsilon = 0.0001)
	{
		return Math.AbsFloat(a - b) <= epsilon;
	}

	//------------------------------------------------------------------------------------------------
	// Remaps value from [inMin, inMax] to [outMin, outMax] and clamps the result.
	static float RemapClamped(float value, float inMin, float inMax, float outMin, float outMax)
	{
		float t = Math.InverseLerp(inMin, inMax, value);
		t = Math.Clamp(t, 0.0, 1.0);
		return Math.Lerp(outMin, outMax, t);
	}

	//------------------------------------------------------------------------------------------------
	// Returns a random point within a 2D circle of the given radius, centered on origin.
	// The Y component of origin is preserved.
	static vector RandomPointInCircle(vector origin, float radius)
	{
		float angle = Math.RandomFloat(0, Math.PI * 2);
		float dist = Math.Sqrt(Math.RandomFloat(0, 1)) * radius;
		float x = origin[0] + Math.Cos(angle) * dist;
		float z = origin[2] + Math.Sin(angle) * dist;
		return Vector(x, origin[1], z);
	}

	//------------------------------------------------------------------------------------------------
	// Returns a random point within a sphere of the given radius, centered on origin.
	static vector RandomPointInSphere(vector origin, float radius)
	{
		float angle1 = Math.RandomFloat(0, Math.PI * 2);
		float angle2 = Math.RandomFloat(0, Math.PI * 2);
		float dist = Math.Pow(Math.RandomFloat(0, 1), 0.333) * radius;

		float x = origin[0] + dist * Math.Sin(angle1) * Math.Cos(angle2);
		float y = origin[1] + dist * Math.Sin(angle1) * Math.Sin(angle2);
		float z = origin[2] + dist * Math.Cos(angle1);

		return Vector(x, y, z);
	}

	//------------------------------------------------------------------------------------------------
	// Wraps value within [min, max] with wraparound (unlike Clamp which caps).
	// e.g. Wrap(370, 0, 360) -> 10
	static float Wrap(float value, float min, float max)
	{
		float range = max - min;
		if (range <= 0)
			return min;

		return value - range * Math.Floor((value - min) / range);
	}

	//------------------------------------------------------------------------------------------------
	// Returns the percentage (0-100) of value within [min, max], clamped.
	static float Percent(float value, float min, float max)
	{
		if (max <= min)
			return 0;

		return Math.Clamp((value - min) / (max - min) * 100.0, 0.0, 100.0);
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if value is within [min, max] inclusive.
	static bool InRange(float value, float min, float max)
	{
		return value >= min && value <= max;
	}

	//------------------------------------------------------------------------------------------------
	// Rounds value to the given number of decimal places.
	static float RoundTo(float value, int decimals)
	{
		float factor = Math.Pow(10, decimals);
		return Math.Round(value * factor) / factor;
	}
}

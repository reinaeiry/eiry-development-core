// Collection utility methods for common array operations that Enfusion's
// array class does not provide natively.
// Note: Enfusion has no generics, so helpers are provided per type.
// The most common types (int, string, IEntity, Managed) are covered.

class EIR_Collections
{
	// ----------------------------------------------------------------
	// Random element
	// ----------------------------------------------------------------

	static IEntity RandomEntity(notnull array<IEntity> arr)
	{
		if (arr.IsEmpty())
			return null;

		return arr[Math.RandomInt(0, arr.Count())];
	}

	static int RandomInt(notnull array<int> arr)
	{
		if (arr.IsEmpty())
			return 0;

		return arr[Math.RandomInt(0, arr.Count())];
	}

	static string RandomString(notnull array<string> arr)
	{
		if (arr.IsEmpty())
			return string.Empty;

		return arr[Math.RandomInt(0, arr.Count())];
	}

	// ----------------------------------------------------------------
	// First / Last
	// ----------------------------------------------------------------

	static IEntity FirstEntity(notnull array<IEntity> arr)
	{
		if (arr.IsEmpty())
			return null;

		return arr[0];
	}

	static IEntity LastEntity(notnull array<IEntity> arr)
	{
		if (arr.IsEmpty())
			return null;

		return arr[arr.Count() - 1];
	}

	static string FirstString(notnull array<string> arr)
	{
		if (arr.IsEmpty())
			return string.Empty;

		return arr[0];
	}

	static string LastString(notnull array<string> arr)
	{
		if (arr.IsEmpty())
			return string.Empty;

		return arr[arr.Count() - 1];
	}

	// ----------------------------------------------------------------
	// Shuffle (Fisher-Yates in-place)
	// ----------------------------------------------------------------

	static void ShuffleEntities(notnull array<IEntity> arr)
	{
		int n = arr.Count();
		for (int i = n - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			IEntity temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
		}
	}

	static void ShuffleInts(notnull array<int> arr)
	{
		int n = arr.Count();
		for (int i = n - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			int temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
		}
	}

	static void ShuffleStrings(notnull array<string> arr)
	{
		int n = arr.Count();
		for (int i = n - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			string temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
		}
	}

	// ----------------------------------------------------------------
	// Contains check with typed versions
	// ----------------------------------------------------------------

	static bool ContainsString(notnull array<string> arr, string value)
	{
		return arr.Find(value) != -1;
	}

	static bool ContainsInt(notnull array<int> arr, int value)
	{
		return arr.Find(value) != -1;
	}

	// ----------------------------------------------------------------
	// Deduplicate
	// ----------------------------------------------------------------

	// Removes duplicate strings from arr in-place.
	static void DeduplicateStrings(notnull array<string> arr)
	{
		for (int i = arr.Count() - 1; i >= 0; i--)
		{
			for (int j = 0; j < i; j++)
			{
				if (arr[j] == arr[i])
				{
					arr.Remove(i);
					break;
				}
			}
		}
	}

	// Removes duplicate ints from arr in-place.
	static void DeduplicateInts(notnull array<int> arr)
	{
		for (int i = arr.Count() - 1; i >= 0; i--)
		{
			for (int j = 0; j < i; j++)
			{
				if (arr[j] == arr[i])
				{
					arr.Remove(i);
					break;
				}
			}
		}
	}
}

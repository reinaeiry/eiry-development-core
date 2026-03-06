// Static string utility methods that complement Enfusion's built-in string
// proto methods and the base game's SCR_StringHelper.
//
// Does not duplicate: string.Split, SCR_StringHelper.Join,
// SCR_StringHelper.TrimLeft, SCR_StringHelper.TrimRight.

class EIR_StringUtils
{
	//------------------------------------------------------------------------------------------------
	// Returns true if str is empty (length 0).
	// Named to mirror the common IsNullOrEmpty pattern — Enfusion strings
	// cannot be null, but can be empty.
	static bool IsEmpty(string str)
	{
		return str.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if str starts with the given prefix.
	static bool StartsWith(string str, string prefix)
	{
		int prefixLen = prefix.Length();
		if (prefixLen == 0)
			return true;

		if (str.Length() < prefixLen)
			return false;

		return str.Substring(0, prefixLen) == prefix;
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if str ends with the given suffix.
	static bool EndsWith(string str, string suffix)
	{
		int suffixLen = suffix.Length();
		if (suffixLen == 0)
			return true;

		int strLen = str.Length();
		if (strLen < suffixLen)
			return false;

		return str.Substring(strLen - suffixLen, suffixLen) == suffix;
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if str contains substr anywhere within it.
	static bool Contains(string str, string substr)
	{
		if (substr.IsEmpty())
			return true;

		return str.Contains(substr);
	}

	//------------------------------------------------------------------------------------------------
	// Strips leading and trailing whitespace (spaces and tabs).
	static string Trim(string str)
	{
		return SCR_StringHelper.TrimLeft(SCR_StringHelper.TrimRight(str));
	}

	//------------------------------------------------------------------------------------------------
	// Pads str on the right with spaces until it reaches targetLength.
	// Returns str unchanged if it is already at or beyond targetLength.
	static string PadRight(string str, int targetLength)
	{
		int len = str.Length();
		if (len >= targetLength)
			return str;

		string result = str;
		int padding = targetLength - len;
		for (int i = 0; i < padding; i++)
		{
			result += " ";
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// Pads str on the left with spaces until it reaches targetLength.
	static string PadLeft(string str, int targetLength)
	{
		int len = str.Length();
		if (len >= targetLength)
			return str;

		string pad = "";
		int padding = targetLength - len;
		for (int i = 0; i < padding; i++)
		{
			pad += " ";
		}

		return pad + str;
	}

	//------------------------------------------------------------------------------------------------
	// Repeats str count times and returns the result.
	static string Repeat(string str, int count)
	{
		string result = "";
		for (int i = 0; i < count; i++)
		{
			result += str;
		}

		return result;
	}
}

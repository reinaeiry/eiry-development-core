// Fluent builder for constructing JSON strings in Enfusion script.
// Enfusion has no built-in JSON serialiser, so this covers the common
// cases needed for NetBus payloads and web API integration.
//
// ----- Object -----
//
//   EIR_JsonBuilder b = EIR_JsonBuilder.NewObject();
//   b.AddString("questId", "sweep_stary");
//   b.AddInt("playerId", 5);
//   b.AddFloat("progress", 0.75);
//   b.AddBool("completed", false);
//   string json = b.Build();
//   // -> {"questId":"sweep_stary","playerId":5,"progress":0.75,"completed":false}
//
// ----- Array -----
//
//   EIR_JsonBuilder arr = EIR_JsonBuilder.NewArray();
//   arr.PushString("alpha");
//   arr.PushString("bravo");
//   arr.PushInt(42);
//   string json = arr.Build();
//   // -> ["alpha","bravo",42]
//
// ----- Nested -----
//
//   EIR_JsonBuilder inner = EIR_JsonBuilder.NewObject();
//   inner.AddInt("x", 10);
//   inner.AddInt("y", 20);
//
//   EIR_JsonBuilder outer = EIR_JsonBuilder.NewObject();
//   outer.AddString("name", "spawn");
//   outer.AddRaw("position", inner.Build());
//   string json = outer.Build();
//   // -> {"name":"spawn","position":{"x":10,"y":20}}

class EIR_JsonBuilder
{
	protected string m_sContent;
	protected bool m_bIsArray;
	protected bool m_bFirst;

	//------------------------------------------------------------------------------------------------
	// Create a builder for a JSON object { }.
	static EIR_JsonBuilder NewObject()
	{
		EIR_JsonBuilder b = new EIR_JsonBuilder();
		b.m_bIsArray = false;
		b.m_bFirst = true;
		return b;
	}

	//------------------------------------------------------------------------------------------------
	// Create a builder for a JSON array [ ].
	static EIR_JsonBuilder NewArray()
	{
		EIR_JsonBuilder b = new EIR_JsonBuilder();
		b.m_bIsArray = true;
		b.m_bFirst = true;
		return b;
	}

	// ----------------------------------------------------------------
	// Object key/value methods
	// ----------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void AddString(string key, string value)
	{
		AppendKey(key);
		m_sContent += "\"" + Escape(value) + "\"";
	}

	//------------------------------------------------------------------------------------------------
	void AddInt(string key, int value)
	{
		AppendKey(key);
		m_sContent += value.ToString();
	}

	//------------------------------------------------------------------------------------------------
	void AddFloat(string key, float value, int decimals = 3)
	{
		AppendKey(key);
		m_sContent += FloatToString(value, decimals);
	}

	//------------------------------------------------------------------------------------------------
	void AddBool(string key, bool value)
	{
		AppendKey(key);
		if (value)
			m_sContent += "true";
		else
			m_sContent += "false";
	}

	//------------------------------------------------------------------------------------------------
	void AddNull(string key)
	{
		AppendKey(key);
		m_sContent += "null";
	}

	//------------------------------------------------------------------------------------------------
	// Add an already-serialised JSON value (object, array, number, etc.) under key.
	void AddRaw(string key, string rawJson)
	{
		AppendKey(key);
		m_sContent += rawJson;
	}

	// ----------------------------------------------------------------
	// Array push methods
	// ----------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void PushString(string value)
	{
		AppendComma();
		m_sContent += "\"" + Escape(value) + "\"";
	}

	//------------------------------------------------------------------------------------------------
	void PushInt(int value)
	{
		AppendComma();
		m_sContent += value.ToString();
	}

	//------------------------------------------------------------------------------------------------
	void PushFloat(float value, int decimals = 3)
	{
		AppendComma();
		m_sContent += FloatToString(value, decimals);
	}

	//------------------------------------------------------------------------------------------------
	void PushBool(bool value)
	{
		AppendComma();
		if (value)
			m_sContent += "true";
		else
			m_sContent += "false";
	}

	//------------------------------------------------------------------------------------------------
	void PushNull()
	{
		AppendComma();
		m_sContent += "null";
	}

	//------------------------------------------------------------------------------------------------
	// Push an already-serialised JSON value into the array.
	void PushRaw(string rawJson)
	{
		AppendComma();
		m_sContent += rawJson;
	}

	// ----------------------------------------------------------------
	// Finalise
	// ----------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	// Returns the completed JSON string.
	string Build()
	{
		if (m_bIsArray)
			return "[" + m_sContent + "]";

		return "{" + m_sContent + "}";
	}

	// ----------------------------------------------------------------
	// Internal helpers
	// ----------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void AppendComma()
	{
		if (!m_bFirst)
			m_sContent += ",";

		m_bFirst = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void AppendKey(string key)
	{
		AppendComma();
		m_sContent += "\"" + Escape(key) + "\":";
	}

	//------------------------------------------------------------------------------------------------
	// Escapes double quotes in string values for JSON compliance.
	// Iterates character by character to avoid multi-backslash string literal issues.
	protected string Escape(string value)
	{
		string result = "";
		int len = value.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = value.Substring(i, 1);
			if (ch == "\"")
				result += "\\" + "\"";
			else
				result += ch;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected string FloatToString(float value, int decimals)
	{
		int factorInt = Math.Round(Math.Pow(10, decimals));
		int rounded = Math.Round(value * factorInt);
		string intPart = (rounded / factorInt).ToString();
		string fracPart = Math.AbsInt(rounded % factorInt).ToString();

		while (fracPart.Length() < decimals)
		{
			fracPart = "0" + fracPart;
		}

		return intPart + "." + fracPart;
	}
}

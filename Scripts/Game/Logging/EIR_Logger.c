// Structured logging utility. Wraps Print() with category prefixes and
// level filtering so you can control output per-system without touching
// call sites.
//
// ----- Basic usage -----
//
//   EIR_Logger.Info("questing", "Player 5 completed sweep_stary");
//   EIR_Logger.Warning("netbus", "No EIR_NetBusComponent found");
//   EIR_Logger.Error("economy", "Balance underflow for player 3");
//
// Output format:  [EIR][category] message
//
// ----- Debug / Verbose -----
//
// Debug and Verbose lines are suppressed by default.
// Enable globally:
//   EIR_Logger.SetDebugEnabled(true);
//
// ----- Silencing a category -----
//
//   EIR_Logger.DisableCategory("questing");
//   EIR_Logger.EnableCategory("questing");
//
// Disabled categories suppress all levels including Warning and Error,
// so only silence categories you are sure you don't need.

class EIR_Logger
{
	protected static bool s_bDebugEnabled = false;
	protected static ref map<string, bool> s_DisabledCategories;

	//------------------------------------------------------------------------------------------------
	// Enable or disable Debug and Verbose output globally.
	static void SetDebugEnabled(bool enabled)
	{
		s_bDebugEnabled = enabled;
	}

	//------------------------------------------------------------------------------------------------
	// Suppress all log output for a category.
	static void DisableCategory(string category)
	{
		if (!s_DisabledCategories)
			s_DisabledCategories = new map<string, bool>();

		s_DisabledCategories.Set(category, true);
	}

	//------------------------------------------------------------------------------------------------
	// Re-enable log output for a previously disabled category.
	static void EnableCategory(string category)
	{
		if (!s_DisabledCategories)
			return;

		s_DisabledCategories.Remove(category);
	}

	//------------------------------------------------------------------------------------------------
	// Detailed tracing — only prints when debug is enabled.
	static void Debug(string category, string message)
	{
		if (!s_bDebugEnabled)
			return;

		if (IsCategoryDisabled(category))
			return;

		Print(Format(category, message), LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	// Verbose tracing — only prints when debug is enabled.
	static void Verbose(string category, string message)
	{
		if (!s_bDebugEnabled)
			return;

		if (IsCategoryDisabled(category))
			return;

		Print(Format(category, message), LogLevel.VERBOSE);
	}

	//------------------------------------------------------------------------------------------------
	// General information — always printed unless category is disabled.
	static void Info(string category, string message)
	{
		if (IsCategoryDisabled(category))
			return;

		Print(Format(category, message), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// Non-fatal problem — always printed unless category is disabled.
	static void Warning(string category, string message)
	{
		if (IsCategoryDisabled(category))
			return;

		Print(Format(category, message), LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	// Fatal or data-corrupting problem — always printed unless category is disabled.
	static void Error(string category, string message)
	{
		if (IsCategoryDisabled(category))
			return;

		Print(Format(category, message), LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsCategoryDisabled(string category)
	{
		if (!s_DisabledCategories)
			return false;

		return s_DisabledCategories.Contains(category);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Format(string category, string message)
	{
		return string.Format("[EIR][%1] %2", category, message);
	}
}

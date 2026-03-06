// Global feature/capability registry. Mods announce themselves at startup
// so other mods can detect their presence without a compile dependency.
//
// ----- Registering (call this in your component EOnInit or equivalent) -----
//
//   EIR_FeatureRegistry.GetInstance().Register("eir.questing", "1.0.0");
//
// ----- Querying -----
//
//   if (EIR_FeatureRegistry.GetInstance().IsRegistered("eir.questing"))
//       // show quest UI, hook into quest events, etc.
//
//   string ver = EIR_FeatureRegistry.GetInstance().GetVersion("eir.questing");
//   // returns "" if the feature is not registered
//
// Feature names are freeform strings. Convention: "author.feature" e.g.
//   "eir.questing", "eir.economy", "rz.missions"
//
// Registering the same feature twice is safe — the second call is ignored.

class EIR_FeatureRegistry
{
	protected static ref EIR_FeatureRegistry s_pInstance;

	// feature name -> version string (empty string if no version given)
	protected ref map<string, string> m_Features;

	//------------------------------------------------------------------------------------------------
	static EIR_FeatureRegistry GetInstance()
	{
		if (!s_pInstance)
			s_pInstance = new EIR_FeatureRegistry();

		return s_pInstance;
	}

	//------------------------------------------------------------------------------------------------
	// Announce that a feature is available. version is optional.
	// Safe to call multiple times — subsequent calls for the same feature
	// name are silently ignored.
	void Register(string featureName, string version = "")
	{
		if (!m_Features)
			m_Features = new map<string, string>();

		if (m_Features.Contains(featureName))
			return;

		m_Features.Set(featureName, version);
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if the named feature has been registered.
	bool IsRegistered(string featureName)
	{
		if (!m_Features)
			return false;

		return m_Features.Contains(featureName);
	}

	//------------------------------------------------------------------------------------------------
	// Returns the version string the feature was registered with.
	// Returns "" if the feature is not registered or was registered without a version.
	string GetVersion(string featureName)
	{
		if (!m_Features)
			return "";

		string version;
		if (!m_Features.Find(featureName, version))
			return "";

		return version;
	}

	//------------------------------------------------------------------------------------------------
	// Fills outFeatures with the names of all currently registered features.
	void GetAllRegistered(out array<string> outFeatures)
	{
		if (!outFeatures)
			outFeatures = {};

		outFeatures.Clear();

		if (!m_Features)
			return;

		foreach (string name, string ver : m_Features)
		{
			outFeatures.Insert(name);
		}
	}
}

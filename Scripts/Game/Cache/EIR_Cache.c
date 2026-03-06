// Internal cache entry. Not for direct use.
class EIR_CacheEntry
{
	ref Managed value;
	float expiresAtMs;

	//------------------------------------------------------------------------------------------------
	void EIR_CacheEntry(Managed val, float expiresAt)
	{
		value = val;
		expiresAtMs = expiresAt;
	}
}

// TTL-based key/value cache for Managed objects.
// Create one per system — this is not a singleton.
//
// ----- Usage -----
//
//   EIR_Cache cache = new EIR_Cache();
//
//   // Store for 30 seconds
//   cache.Set("player.5.data", myDataObject, 30.0);
//
//   // Retrieve (returns null if expired or not found)
//   MyDataClass data = MyDataClass.Cast(cache.Get("player.5.data"));
//
//   // Check without retrieval
//   if (cache.Has("player.5.data")) { ... }
//
//   // Invalidate manually
//   cache.Remove("player.5.data");
//   cache.Clear();
//
// Values are only evicted when accessed. Call Purge() periodically
// if memory usage from expired entries is a concern.

class EIR_Cache
{
	protected ref map<string, ref EIR_CacheEntry> m_Entries;

	//------------------------------------------------------------------------------------------------
	// Store value under key with a TTL in seconds.
	// Overwrites any existing entry for the same key.
	void Set(string key, Managed value, float ttlSeconds)
	{
		if (!m_Entries)
			m_Entries = new map<string, ref EIR_CacheEntry>();

		float expiresAt = GetGame().GetWorld().GetWorldTime() + ttlSeconds * 1000.0;
		m_Entries.Set(key, new EIR_CacheEntry(value, expiresAt));
	}

	//------------------------------------------------------------------------------------------------
	// Returns the cached value, or null if not present or expired.
	Managed Get(string key)
	{
		if (!m_Entries)
			return null;

		EIR_CacheEntry entry = m_Entries.Get(key);
		if (!entry)
			return null;

		if (GetGame().GetWorld().GetWorldTime() > entry.expiresAtMs)
		{
			m_Entries.Remove(key);
			return null;
		}

		return entry.value;
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if the key exists and has not expired.
	bool Has(string key)
	{
		return Get(key) != null;
	}

	//------------------------------------------------------------------------------------------------
	// Remove a specific key. Safe to call if key does not exist.
	void Remove(string key)
	{
		if (!m_Entries)
			return;

		m_Entries.Remove(key);
	}

	//------------------------------------------------------------------------------------------------
	// Remove all entries.
	void Clear()
	{
		if (m_Entries)
			m_Entries.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// Remove all entries that have already expired. Call periodically
	// if you have many entries that expire without being accessed.
	void Purge()
	{
		if (!m_Entries)
			return;

		float now = GetGame().GetWorld().GetWorldTime();
		array<string> toRemove = {};

		foreach (string key, EIR_CacheEntry entry : m_Entries)
		{
			if (now > entry.expiresAtMs)
				toRemove.Insert(key);
		}

		foreach (string key : toRemove)
		{
			m_Entries.Remove(key);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Returns the number of entries (including expired ones not yet purged).
	int Count()
	{
		if (!m_Entries)
			return 0;

		return m_Entries.Count();
	}
}

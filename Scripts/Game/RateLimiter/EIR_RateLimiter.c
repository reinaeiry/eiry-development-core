// Per-key rate limiter. Prevents the same action from firing more often
// than a configured cooldown allows.
//
// Create one per system with its own cooldown requirement.
// Keys are freeform strings — typically playerId, "playerId.eventName", etc.
//
// ----- Usage -----
//
//   // One limiter per notification type, shared across all players.
//   EIR_RateLimiter notifLimiter = new EIR_RateLimiter(5.0); // 5s cooldown
//
//   // In your notification send path:
//   string key = string.ToString(playerId);
//   if (!notifLimiter.Allow(key))
//       return;  // still on cooldown for this player
//
//   // Proceed to send notification...
//
// ----- Per-event cooldowns -----
//
//   // Use a compound key to rate-limit per player per event type.
//   string key = string.Format("%1.%2", playerId, eventName);
//   if (!limiter.Allow(key)) return;

class EIR_RateLimiter
{
	protected float m_fCooldownSeconds;
	protected ref map<string, float> m_LastAllowedTimes;

	//------------------------------------------------------------------------------------------------
	void EIR_RateLimiter(float cooldownSeconds)
	{
		m_fCooldownSeconds = cooldownSeconds;
		m_LastAllowedTimes = new map<string, float>();
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if the action is permitted for key, and records the time.
	// Returns false if key is still within the cooldown window.
	bool Allow(string key)
	{
		float nowSeconds = GetGame().GetWorld().GetWorldTime() / 1000.0;

		float lastTime = 0;
		m_LastAllowedTimes.Find(key, lastTime);

		if (nowSeconds - lastTime < m_fCooldownSeconds)
			return false;

		m_LastAllowedTimes.Set(key, nowSeconds);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if key is currently within the cooldown window (i.e. Allow would return false).
	bool IsLimited(string key)
	{
		float nowSeconds = GetGame().GetWorld().GetWorldTime() / 1000.0;

		float lastTime = 0;
		m_LastAllowedTimes.Find(key, lastTime);

		return nowSeconds - lastTime < m_fCooldownSeconds;
	}

	//------------------------------------------------------------------------------------------------
	// Returns how many seconds remain on the cooldown for key.
	// Returns 0 if the key is not limited.
	float GetRemainingCooldown(string key)
	{
		float nowSeconds = GetGame().GetWorld().GetWorldTime() / 1000.0;

		float lastTime = 0;
		m_LastAllowedTimes.Find(key, lastTime);

		float remaining = m_fCooldownSeconds - (nowSeconds - lastTime);
		if (remaining < 0)
			return 0;

		return remaining;
	}

	//------------------------------------------------------------------------------------------------
	// Reset the cooldown for a specific key, allowing it to fire immediately.
	void Reset(string key)
	{
		m_LastAllowedTimes.Remove(key);
	}

	//------------------------------------------------------------------------------------------------
	// Reset all cooldowns.
	void ResetAll()
	{
		m_LastAllowedTimes.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// Change the cooldown duration. Does not affect already-running cooldowns retroactively.
	void SetCooldown(float cooldownSeconds)
	{
		m_fCooldownSeconds = cooldownSeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetCooldown()
	{
		return m_fCooldownSeconds;
	}
}

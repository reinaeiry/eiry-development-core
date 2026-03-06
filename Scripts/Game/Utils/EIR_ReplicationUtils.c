// Clean static wrappers for the repetitive Replication / RplSession checks.
// Reduces noise and makes intent explicit at call sites.
//
// Usage:
//   if (EIR_ReplicationUtils.IsServer()) { ... }
//   if (EIR_ReplicationUtils.IsDedicatedServer()) { ... }
//   if (EIR_ReplicationUtils.IsListenServer()) { ... }

class EIR_ReplicationUtils
{
	//------------------------------------------------------------------------------------------------
	// True on the server peer (both dedicated and listen server).
	static bool IsServer()
	{
		return Replication.IsServer();
	}

	//------------------------------------------------------------------------------------------------
	// True only on a dedicated server (no local player).
	static bool IsDedicatedServer()
	{
		return RplSession.Mode() == RplMode.Dedicated;
	}

	//------------------------------------------------------------------------------------------------
	// True when running as a listen server — the host is both server and a client.
	static bool IsListenServer()
	{
		return Replication.IsServer() && RplSession.Mode() != RplMode.Dedicated;
	}

	//------------------------------------------------------------------------------------------------
	// True on client peers (not server). False on any server peer.
	static bool IsClient()
	{
		return !Replication.IsServer();
	}

	//------------------------------------------------------------------------------------------------
	// True if replication is active at all (multiplayer session).
	// False in singleplayer or local editor play.
	static bool IsMultiplayer()
	{
		return RplSession.Mode() != RplMode.None;
	}

	//------------------------------------------------------------------------------------------------
	// Returns a human-readable string for the current peer role.
	// Useful for log messages.
	static string GetPeerRole()
	{
		if (IsDedicatedServer())
			return "DedicatedServer";

		if (IsListenServer())
			return "ListenServer";

		if (IsClient())
			return "Client";

		return "Singleplayer";
	}
}

// Static utility methods for common player resolution boilerplate.
// Every mod needs these — centralised here so they're not rewritten each time.

class EIR_PlayerUtils
{
	//------------------------------------------------------------------------------------------------
	// Returns true if playerId is a positive non-zero value (basic sanity check).
	// Does not verify the player is actually connected.
	static bool IsValidId(int playerId)
	{
		return playerId > 0;
	}

	//------------------------------------------------------------------------------------------------
	// Returns the entity currently controlled by the given player, or null.
	static IEntity GetEntityFromId(int playerId)
	{
		if (playerId <= 0)
			return null;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return null;

		return pm.GetPlayerControlledEntity(playerId);
	}

	//------------------------------------------------------------------------------------------------
	// Returns the display name of the given player, or an empty string if not found.
	static string GetName(int playerId)
	{
		if (playerId <= 0)
			return string.Empty;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return string.Empty;

		return pm.GetPlayerName(playerId);
	}

	//------------------------------------------------------------------------------------------------
	// Returns a formatted "Name (id)" string useful for log messages.
	static string Describe(int playerId)
	{
		string name = GetName(playerId);
		if (name.IsEmpty())
			return string.Format("(id:%1)", playerId);

		return string.Format("%1 (id:%2)", name, playerId);
	}

	//------------------------------------------------------------------------------------------------
	// Fills outPlayers with the IDs of all currently connected players.
	// Returns the count.
	static int GetAllConnectedIds(out array<int> outPlayers)
	{
		if (!outPlayers)
			outPlayers = {};

		outPlayers.Clear();

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return 0;

		return pm.GetPlayers(outPlayers);
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if the given entity is currently controlled by any player,
	// and writes that player's ID into outPlayerId.
	static bool TryGetIdFromEntity(IEntity entity, out int outPlayerId)
	{
		outPlayerId = -1;

		if (!entity)
			return false;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return false;

		array<int> players = {};
		pm.GetPlayers(players);

		foreach (int id : players)
		{
			if (pm.GetPlayerControlledEntity(id) == entity)
			{
				outPlayerId = id;
				return true;
			}
		}

		return false;
	}
}

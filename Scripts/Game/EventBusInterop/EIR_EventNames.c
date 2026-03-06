// Standard event name constants for EIR_EventBus.
// Use these instead of raw strings to avoid typos and keep names in one place.
//
// Convention for custom events: "author.category.action"
// e.g. "myaddon.economy.currency.changed"

class EIR_EventNames
{
	// ----------------------------------------------------------------
	// Player lifecycle
	// ----------------------------------------------------------------

	// Fired when a player connects and their data has been loaded.
	// Payload: none (use playerId from your own context, or extend EIR_EventPayload)
	static const string PLAYER_CONNECTED		= "eir.player.connected";

	// Fired when a player disconnects after their data has been saved.
	static const string PLAYER_DISCONNECTED		= "eir.player.disconnected";

	// ----------------------------------------------------------------
	// Quest events
	// ----------------------------------------------------------------

	// Fired when a player starts a new quest.
	static const string QUEST_STARTED			= "eir.quest.started";

	// Fired when a player makes progress on an active quest objective.
	static const string QUEST_PROGRESS			= "eir.quest.progress";

	// Fired when a player completes a quest.
	static const string QUEST_COMPLETED			= "eir.quest.completed";

	// Fired when a player's quest is abandoned or reset.
	static const string QUEST_ABANDONED			= "eir.quest.abandoned";

	// ----------------------------------------------------------------
	// Server lifecycle
	// ----------------------------------------------------------------

	// Fired once the server has finished initialising all core systems.
	static const string SERVER_READY			= "eir.server.ready";

	// ----------------------------------------------------------------
	// Economy (reserved for a future economy system)
	// ----------------------------------------------------------------

	// Fired when a player's currency balance changes.
	static const string ECONOMY_BALANCE_CHANGED	= "eir.economy.balance.changed";

	// Fired when a player completes a purchase.
	static const string ECONOMY_PURCHASE		= "eir.economy.purchase";
}

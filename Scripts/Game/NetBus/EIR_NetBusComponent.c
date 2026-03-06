[ComponentEditorProps(category: "EIR/Core", description: "Gateway RPC host for EIR_NetBus. Place on the GameMode entity.")]
class EIR_NetBusComponentClass : ScriptComponentClass
{
}

// RplComponent that owns the three gateway RPCs through which all
// EIR_NetBus messages flow. Attach one instance to the GameMode entity.
//
// This component has no configurable attributes — it is purely a host for
// the replication methods required by Enfusion's compile-time RPC system.
//
// Access via EIR_NetBusComponent.GetInstance() from any script on any peer.

class EIR_NetBusComponent : ScriptComponent
{
	protected static EIR_NetBusComponent s_pInstance;

	//------------------------------------------------------------------------------------------------
	void EIR_NetBusComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		s_pInstance = this;
	}

	//------------------------------------------------------------------------------------------------
	static EIR_NetBusComponent GetInstance()
	{
		if (!s_pInstance)
			Print("[EIR_NetBus] GetInstance: no EIR_NetBusComponent found — place it on the GameMode entity.", LogLevel.WARNING);

		return s_pInstance;
	}

	// ================================================================
	// SERVER → CLIENT
	// ================================================================

	//------------------------------------------------------------------------------------------------
	// Send a message to a specific player. Call from server only.
	void SendToPlayer(int playerId, string typeId, string payload)
	{
		Rpc(Rpc_ToPlayer, playerId, typeId, payload);
	}

	//------------------------------------------------------------------------------------------------
	// Send a message to all connected clients. Call from server only.
	void SendToAll(string typeId, string payload)
	{
		Rpc(Rpc_ToAll, typeId, payload);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_ToPlayer(int playerId, string typeId, string payload)
	{
		int localId = SCR_PlayerController.GetLocalPlayerId();
		if (localId != playerId)
			return;

		EIR_NetBus.GetInstance().DispatchToClient(typeId, payload, -1);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_ToAll(string typeId, string payload)
	{
		EIR_NetBus.GetInstance().DispatchToClient(typeId, payload, -1);
	}

	// ================================================================
	// CLIENT → SERVER
	// ================================================================

	//------------------------------------------------------------------------------------------------
	// Send a message to the server. Call from client only.
	// On a listen server host, routes directly without a network hop.
	void SendToServer(string typeId, string payload)
	{
		if (Replication.IsServer())
		{
			int localId = SCR_PlayerController.GetLocalPlayerId();
			EIR_NetBus.GetInstance().DispatchToServer(typeId, payload, localId);
			return;
		}

		int localId = SCR_PlayerController.GetLocalPlayerId();
		Rpc(Rpc_ToServer, localId, typeId, payload);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_ToServer(int fromPlayerId, string typeId, string payload)
	{
		EIR_NetBus.GetInstance().DispatchToServer(typeId, payload, fromPlayerId);
	}
}

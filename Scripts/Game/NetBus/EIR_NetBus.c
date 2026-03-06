// Handler callback signature for incoming messages.
// typeId    — the string identifier the sender used
// payload   — freeform string data (format agreed between sender and receiver)
// fromId    — the playerId of the sender; -1 when sent from the server
void EIR_NetHandler(string typeId, string payload, int fromId);
typedef func EIR_NetHandler;
typedef ScriptInvokerBase<EIR_NetHandler> EIR_NetHandlerInvoker;

// Central routing table for EIR_NetBusComponent messages.
// EIR_NetBusComponent calls DispatchToClient / DispatchToServer when gateway
// RPCs arrive; everything else registers and fires through here.
//
// ----- Sending (server → client) -----
//   EIR_NetBusComponent comp = EIR_NetBusComponent.GetInstance();
//   comp.SendToPlayer(playerId, "quest.update", payload);
//   comp.SendToAll("server.event", payload);
//
// ----- Sending (client → server) -----
//   EIR_NetBusComponent.GetInstance().SendToServer("player.action", payload);
//
// ----- Receiving on the client -----
//   // In your client-side component init:
//   EIR_NetBus.GetInstance().RegisterClientHandler("quest.update", this.OnQuestUpdate);
//
//   void OnQuestUpdate(string typeId, string payload, int fromId)
//   {
//       // parse payload, update UI, etc.
//   }
//
// ----- Receiving on the server -----
//   EIR_NetBus.GetInstance().RegisterServerHandler("player.action", this.OnPlayerAction);
//
//   void OnPlayerAction(string typeId, string payload, int fromId)
//   {
//       // fromId is the playerId of the client who sent this
//   }
//
// Multiple handlers may be registered for the same typeId — all are called.
// Registering the same handler twice will call it twice per dispatch.

class EIR_NetBus
{
	protected static ref EIR_NetBus s_pInstance;

	// Handlers called when a message arrives on the client (sent from server).
	protected ref map<string, ref EIR_NetHandlerInvoker> m_ClientHandlers;

	// Handlers called when a message arrives on the server (sent from a client).
	protected ref map<string, ref EIR_NetHandlerInvoker> m_ServerHandlers;

	//------------------------------------------------------------------------------------------------
	static EIR_NetBus GetInstance()
	{
		if (!s_pInstance)
			s_pInstance = new EIR_NetBus();

		return s_pInstance;
	}

	//------------------------------------------------------------------------------------------------
	// Register a handler called when this typeId is received on the client.
	void RegisterClientHandler(string typeId, EIR_NetHandler callback)
	{
		RegisterInMap(m_ClientHandlers, typeId, callback);
	}

	//------------------------------------------------------------------------------------------------
	// Register a handler called when this typeId is received on the server.
	void RegisterServerHandler(string typeId, EIR_NetHandler callback)
	{
		RegisterInMap(m_ServerHandlers, typeId, callback);
	}

	//------------------------------------------------------------------------------------------------
	// Remove a previously registered client-side handler.
	void UnregisterClientHandler(string typeId, EIR_NetHandler callback)
	{
		UnregisterFromMap(m_ClientHandlers, typeId, callback);
	}

	//------------------------------------------------------------------------------------------------
	// Remove a previously registered server-side handler.
	void UnregisterServerHandler(string typeId, EIR_NetHandler callback)
	{
		UnregisterFromMap(m_ServerHandlers, typeId, callback);
	}

	//------------------------------------------------------------------------------------------------
	// Called by EIR_NetBusComponent when a server→client message arrives on this peer.
	void DispatchToClient(string typeId, string payload, int fromId)
	{
		Dispatch(m_ClientHandlers, typeId, payload, fromId);
	}

	//------------------------------------------------------------------------------------------------
	// Called by EIR_NetBusComponent when a client→server message arrives on the server.
	void DispatchToServer(string typeId, string payload, int fromId)
	{
		Dispatch(m_ServerHandlers, typeId, payload, fromId);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterInMap(out map<string, ref EIR_NetHandlerInvoker> handlerMap, string typeId, EIR_NetHandler callback)
	{
		if (!handlerMap)
			handlerMap = new map<string, ref EIR_NetHandlerInvoker>();

		EIR_NetHandlerInvoker invoker = handlerMap.Get(typeId);
		if (!invoker)
		{
			invoker = new EIR_NetHandlerInvoker();
			handlerMap.Set(typeId, invoker);
		}

		invoker.Insert(callback);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterFromMap(map<string, ref EIR_NetHandlerInvoker> handlerMap, string typeId, EIR_NetHandler callback)
	{
		if (!handlerMap)
			return;

		EIR_NetHandlerInvoker invoker = handlerMap.Get(typeId);
		if (!invoker)
			return;

		invoker.Remove(callback);
	}

	//------------------------------------------------------------------------------------------------
	protected void Dispatch(map<string, ref EIR_NetHandlerInvoker> handlerMap, string typeId, string payload, int fromId)
	{
		if (!handlerMap)
			return;

		EIR_NetHandlerInvoker invoker = handlerMap.Get(typeId);
		if (!invoker)
			return;

		invoker.Invoke(typeId, payload, fromId);
	}
}

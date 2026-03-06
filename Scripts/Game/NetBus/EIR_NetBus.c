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
//   EIR_NetBus.GetInstance().GetClientInvoker("quest.update").Insert(this.OnQuestUpdate);
//
//   void OnQuestUpdate(string typeId, string payload, int fromId)
//   {
//       // parse payload, update UI, etc.
//   }
//
// ----- Receiving on the server -----
//   EIR_NetBus.GetInstance().GetServerInvoker("player.action").Insert(this.OnPlayerAction);
//
//   void OnPlayerAction(string typeId, string payload, int fromId)
//   {
//       // fromId is the playerId of the client who sent this
//   }
//
// Multiple handlers may be registered for the same typeId — all are called.

class EIR_NetBus
{
	protected static ref EIR_NetBus s_pInstance;

	// Invokers called when a message arrives on the client (sent from server).
	protected ref map<string, ref EIR_NetHandlerInvoker> m_ClientHandlers;

	// Invokers called when a message arrives on the server (sent from a client).
	protected ref map<string, ref EIR_NetHandlerInvoker> m_ServerHandlers;

	//------------------------------------------------------------------------------------------------
	static EIR_NetBus GetInstance()
	{
		if (!s_pInstance)
			s_pInstance = new EIR_NetBus();

		return s_pInstance;
	}

	//------------------------------------------------------------------------------------------------
	// Returns the client-side invoker for typeId, creating it if needed.
	// Use .Insert(this.MyCallback) / .Remove(this.MyCallback) to manage subscriptions.
	EIR_NetHandlerInvoker GetClientInvoker(string typeId)
	{
		return GetOrCreateInvoker(m_ClientHandlers, typeId);
	}

	//------------------------------------------------------------------------------------------------
	// Returns the server-side invoker for typeId, creating it if needed.
	EIR_NetHandlerInvoker GetServerInvoker(string typeId)
	{
		return GetOrCreateInvoker(m_ServerHandlers, typeId);
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
	protected EIR_NetHandlerInvoker GetOrCreateInvoker(out map<string, ref EIR_NetHandlerInvoker> handlerMap, string typeId)
	{
		if (!handlerMap)
			handlerMap = new map<string, ref EIR_NetHandlerInvoker>();

		EIR_NetHandlerInvoker invoker = handlerMap.Get(typeId);
		if (!invoker)
		{
			invoker = new EIR_NetHandlerInvoker();
			handlerMap.Set(typeId, invoker);
		}

		return invoker;
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

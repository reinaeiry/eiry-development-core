// Callback signature — all subscribers must match this exactly.
void EIR_EventCallback(string eventName, EIR_EventPayload payload);
typedef func EIR_EventCallback;
typedef ScriptInvokerBase<EIR_EventCallback> EIR_EventInvoker;

// Global pub/sub event bus. Any script can fire named events and any
// other script can subscribe to them with no direct compile dependency
// between the two sides.
//
// ----- Firing an event -----
//
//   EIR_MyPayload p = new EIR_MyPayload();
//   p.someValue = 42;
//   EIR_EventBus.GetInstance().Fire("my.event", p);
//
//   // No data needed? Pass null.
//   EIR_EventBus.GetInstance().Fire("server.ready");
//
// ----- Subscribing -----
//
//   EIR_EventBus.GetInstance().GetInvoker("my.event").Insert(this.OnMyEvent);
//
//   void OnMyEvent(string eventName, EIR_EventPayload payload)
//   {
//       EIR_MyPayload mp = EIR_MyPayload.Cast(payload);
//       if (!mp)
//           return;
//       // use mp.someValue
//   }
//
// ----- Unsubscribing -----
//
//   EIR_EventBus.GetInstance().GetInvoker("my.event").Remove(this.OnMyEvent);
//
// Firing an event with no subscribers is safe — nothing happens.
// Subscribers are called synchronously in the order they were inserted.

class EIR_EventBus
{
	protected static ref EIR_EventBus s_pInstance;
	protected ref map<string, ref EIR_EventInvoker> m_Invokers;

	//------------------------------------------------------------------------------------------------
	static EIR_EventBus GetInstance()
	{
		if (!s_pInstance)
			s_pInstance = new EIR_EventBus();

		return s_pInstance;
	}

	//------------------------------------------------------------------------------------------------
	// Returns the invoker for the named event, creating it if it does not exist.
	// Use .Insert(this.MyCallback) to subscribe and .Remove(this.MyCallback) to unsubscribe.
	EIR_EventInvoker GetInvoker(string eventName)
	{
		if (!m_Invokers)
			m_Invokers = new map<string, ref EIR_EventInvoker>();

		EIR_EventInvoker invoker = m_Invokers.Get(eventName);
		if (!invoker)
		{
			invoker = new EIR_EventInvoker();
			m_Invokers.Set(eventName, invoker);
		}

		return invoker;
	}

	//------------------------------------------------------------------------------------------------
	// Fire a named event, calling all subscribers synchronously.
	// payload may be null when the event carries no data.
	// Safe to call when nobody is subscribed.
	void Fire(string eventName, EIR_EventPayload payload = null)
	{
		if (!m_Invokers)
			return;

		EIR_EventInvoker invoker = m_Invokers.Get(eventName);
		if (!invoker)
			return;

		invoker.Invoke(eventName, payload);
	}
}

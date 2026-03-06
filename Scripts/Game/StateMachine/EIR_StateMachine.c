// Callback signature for state enter/exit events.
// prevState — state being left  (empty string on initial entry)
// nextState — state being entered
void EIR_StateCallback(string prevState, string nextState);
typedef func EIR_StateCallback;
typedef ScriptInvokerBase<EIR_StateCallback> EIR_StateInvoker;

// A single named state. Retrieve via EIR_StateMachine.RegisterState().
// Insert callbacks on GetOnEnter() and GetOnExit() before calling Transition().
class EIR_StateEntry
{
	protected string m_sName;
	protected ref EIR_StateInvoker m_OnEnter;
	protected ref EIR_StateInvoker m_OnExit;

	//------------------------------------------------------------------------------------------------
	void EIR_StateEntry(string name)
	{
		m_sName = name;
		m_OnEnter = new EIR_StateInvoker();
		m_OnExit = new EIR_StateInvoker();
	}

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	// Insert callbacks here to be notified when this state is entered.
	// Signature: void MyCallback(string prevState, string nextState)
	EIR_StateInvoker GetOnEnter()
	{
		return m_OnEnter;
	}

	//------------------------------------------------------------------------------------------------
	// Insert callbacks here to be notified when this state is exited.
	// Signature: void MyCallback(string prevState, string nextState)
	EIR_StateInvoker GetOnExit()
	{
		return m_OnExit;
	}

	//------------------------------------------------------------------------------------------------
	void FireEnter(string prevState)
	{
		if (m_OnEnter)
			m_OnEnter.Invoke(prevState, m_sName);
	}

	//------------------------------------------------------------------------------------------------
	void FireExit(string nextState)
	{
		if (m_OnExit)
			m_OnExit.Invoke(m_sName, nextState);
	}
}

// Simple named-state machine. Create one per system that needs managed state.
//
// ----- Setup -----
//
//   EIR_StateMachine fsm = new EIR_StateMachine();
//
//   EIR_StateEntry idle = fsm.RegisterState("idle");
//   idle.GetOnEnter().Insert(this.OnEnterIdle);
//   idle.GetOnExit().Insert(this.OnExitIdle);
//
//   EIR_StateEntry combat = fsm.RegisterState("combat");
//   combat.GetOnEnter().Insert(this.OnEnterCombat);
//
// ----- Transitioning -----
//
//   fsm.Transition("idle");    // fires OnEnterIdle("", "idle")
//   fsm.Transition("combat");  // fires OnExitIdle("idle","combat"), OnEnterCombat("idle","combat")
//
// ----- Querying -----
//
//   string state = fsm.GetCurrentState();
//   if (fsm.IsInState("combat")) { ... }
//
// Transitioning to the current state is a no-op.
// Transitioning to an unregistered state logs a warning and does nothing.

class EIR_StateMachine
{
	protected ref map<string, ref EIR_StateEntry> m_States;
	protected string m_sCurrentState;

	//------------------------------------------------------------------------------------------------
	// Register a state and return its entry so you can wire up callbacks.
	EIR_StateEntry RegisterState(string name)
	{
		if (!m_States)
			m_States = new map<string, ref EIR_StateEntry>();

		EIR_StateEntry entry = new EIR_StateEntry(name);
		m_States.Set(name, entry);
		return entry;
	}

	//------------------------------------------------------------------------------------------------
	// Transition to newState. Fires the current state's OnExit then newState's OnEnter.
	// Returns false if newState is not registered or is already current.
	bool Transition(string newState)
	{
		if (newState == m_sCurrentState)
			return false;

		if (!m_States || !m_States.Contains(newState))
		{
			EIR_Logger.Warning("statemachine", string.Format("Transition to unregistered state '%1' ignored.", newState));
			return false;
		}

		string prev = m_sCurrentState;

		if (prev != string.Empty && m_States.Contains(prev))
			m_States.Get(prev).FireExit(newState);

		m_sCurrentState = newState;
		m_States.Get(newState).FireEnter(prev);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	string GetCurrentState()
	{
		return m_sCurrentState;
	}

	//------------------------------------------------------------------------------------------------
	bool IsInState(string name)
	{
		return m_sCurrentState == name;
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if the named state has been registered.
	bool HasState(string name)
	{
		if (!m_States)
			return false;

		return m_States.Contains(name);
	}
}

// Callback signature for timer callbacks — no parameters.
void EIR_TimerCallback();
typedef func EIR_TimerCallback;
typedef ScriptInvokerBase<EIR_TimerCallback> EIR_TimerInvoker;

// Internal entry used by EIR_TimerManager. Do not use directly.
class EIR_TimerEntry
{
	protected string m_sName;
	protected ref EIR_TimerInvoker m_Invoker;
	protected bool m_bRepeat;
	protected EIR_TimerManager m_Manager;

	//------------------------------------------------------------------------------------------------
	void EIR_TimerEntry(string name, EIR_TimerInvoker invoker, bool repeat, EIR_TimerManager manager)
	{
		m_sName = name;
		m_Invoker = invoker;
		m_bRepeat = repeat;
		m_Manager = manager;
	}

	//------------------------------------------------------------------------------------------------
	// Called by CallQueue on each tick. Fires the invoker and cleans up
	// one-shot entries from the manager after they execute.
	void Fire()
	{
		if (m_Invoker)
			m_Invoker.Invoke();

		if (!m_bRepeat && m_Manager)
			m_Manager.OnEntryFired(m_sName);
	}

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
}

// Named timer system that wraps GetGame().GetCallqueue().
//
// Compared to using CallLater directly:
//   - Cancel by name, no need to hold a func reference
//   - Check if a named timer is already scheduled (IsScheduled)
//   - Scheduling the same name twice replaces the first timer
//
// ----- One-shot timer -----
//
//   EIR_TimerInvoker inv = new EIR_TimerInvoker();
//   inv.Insert(this.OnDelayedAction);
//   EIR_TimerManager.GetInstance().Schedule("my.delay", inv, 5.0);
//
// ----- Repeating timer -----
//
//   EIR_TimerInvoker inv = new EIR_TimerInvoker();
//   inv.Insert(this.OnTick);
//   EIR_TimerManager.GetInstance().Schedule("my.tick", inv, 1.0, true);
//
// ----- Cancel -----
//
//   EIR_TimerManager.GetInstance().Cancel("my.delay");
//
// ----- Check -----
//
//   if (EIR_TimerManager.GetInstance().IsScheduled("my.delay")) { ... }
//
// The callback method must have no parameters:
//   void OnDelayedAction() { ... }

class EIR_TimerManager
{
	protected static ref EIR_TimerManager s_pInstance;
	protected ref map<string, ref EIR_TimerEntry> m_Entries;

	//------------------------------------------------------------------------------------------------
	static EIR_TimerManager GetInstance()
	{
		if (!s_pInstance)
			s_pInstance = new EIR_TimerManager();

		return s_pInstance;
	}

	//------------------------------------------------------------------------------------------------
	// Schedule a timer. If a timer with this name already exists it is cancelled first.
	// delaySeconds — delay before first fire (and between repeats for repeating timers)
	// repeat       — true for a recurring timer, false for a one-shot
	void Schedule(string name, EIR_TimerInvoker invoker, float delaySeconds, bool repeat = false)
	{
		Cancel(name);

		if (!m_Entries)
			m_Entries = new map<string, ref EIR_TimerEntry>();

		EIR_TimerEntry entry = new EIR_TimerEntry(name, invoker, repeat, this);
		m_Entries.Set(name, entry);

		int delayMs = Math.Round(delaySeconds * 1000.0);
		GetGame().GetCallqueue().CallLater(entry.Fire, delayMs, repeat);
	}

	//------------------------------------------------------------------------------------------------
	// Cancel a scheduled timer by name. Safe to call if the name is not scheduled.
	void Cancel(string name)
	{
		if (!m_Entries)
			return;

		EIR_TimerEntry entry = m_Entries.Get(name);
		if (!entry)
			return;

		GetGame().GetCallqueue().Remove(entry.Fire);
		m_Entries.Remove(name);
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if a timer with this name is currently scheduled.
	bool IsScheduled(string name)
	{
		if (!m_Entries)
			return false;

		return m_Entries.Contains(name);
	}

	//------------------------------------------------------------------------------------------------
	// Called by EIR_TimerEntry after a one-shot timer fires, to clean up the registry.
	void OnEntryFired(string name)
	{
		if (!m_Entries)
			return;

		m_Entries.Remove(name);
	}
}

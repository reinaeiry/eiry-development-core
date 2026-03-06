// A single step in an EIR_Sequence. Retrieve via EIR_Sequence.AddStep().
// Insert your callback on GetInvoker() before calling EIR_Sequence.Start().
class EIR_SequenceStep
{
	protected float m_fDelay;
	protected ref EIR_TimerInvoker m_Invoker;

	//------------------------------------------------------------------------------------------------
	void EIR_SequenceStep(float delaySeconds)
	{
		m_fDelay = delaySeconds;
		m_Invoker = new EIR_TimerInvoker();
	}

	//------------------------------------------------------------------------------------------------
	float GetDelay()
	{
		return m_fDelay;
	}

	//------------------------------------------------------------------------------------------------
	// Insert your callback here. Signature: void MyCallback()
	EIR_TimerInvoker GetInvoker()
	{
		return m_Invoker;
	}

	//------------------------------------------------------------------------------------------------
	void Fire()
	{
		if (m_Invoker)
			m_Invoker.Invoke();
	}
}

// Chains a series of timed actions. Steps execute in order, each after
// its configured delay has elapsed since the previous step completed.
//
// ----- Usage -----
//
//   EIR_Sequence seq = new EIR_Sequence();
//
//   EIR_SequenceStep step1 = seq.AddStep(0);        // fires immediately on Start()
//   step1.GetInvoker().Insert(this.DoPhaseOne);
//
//   EIR_SequenceStep step2 = seq.AddStep(3.0);      // fires 3s after step1
//   step2.GetInvoker().Insert(this.DoPhaseTwo);
//
//   EIR_SequenceStep step3 = seq.AddStep(1.5);      // fires 1.5s after step2
//   step3.GetInvoker().Insert(this.DoPhaseThree);
//
//   seq.Start();
//
// ----- Cancelling -----
//
//   seq.Stop();   // cancels any pending step; already-fired steps are not undone
//
// Each EIR_Sequence instance is independent. Create one per use case.
// Calling Start() on a running sequence restarts it from step 0.

class EIR_Sequence
{
	protected ref array<ref EIR_SequenceStep> m_Steps;
	protected int m_iCurrentStep;
	protected bool m_bRunning;

	//------------------------------------------------------------------------------------------------
	// Add a step to the sequence. delaySeconds is the wait after the previous step.
	// Returns the step so you can insert callbacks on it.
	EIR_SequenceStep AddStep(float delaySeconds = 0)
	{
		if (!m_Steps)
			m_Steps = {};

		EIR_SequenceStep step = new EIR_SequenceStep(delaySeconds);
		m_Steps.Insert(step);
		return step;
	}

	//------------------------------------------------------------------------------------------------
	// Begin executing steps from the beginning.
	// If already running, restarts from step 0.
	void Start()
	{
		Stop();

		if (!m_Steps || m_Steps.IsEmpty())
			return;

		m_iCurrentStep = 0;
		m_bRunning = true;
		ScheduleCurrentStep();
	}

	//------------------------------------------------------------------------------------------------
	// Cancel any pending step. Already-fired steps are not undone.
	void Stop()
	{
		if (!m_bRunning)
			return;

		GetGame().GetCallqueue().Remove(OnStepFired);
		m_bRunning = false;
	}

	//------------------------------------------------------------------------------------------------
	bool IsRunning()
	{
		return m_bRunning;
	}

	//------------------------------------------------------------------------------------------------
	// Returns the index of the step that will fire next (0-based).
	int GetCurrentStepIndex()
	{
		return m_iCurrentStep;
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleCurrentStep()
	{
		if (!m_Steps || m_iCurrentStep >= m_Steps.Count())
		{
			m_bRunning = false;
			return;
		}

		EIR_SequenceStep step = m_Steps[m_iCurrentStep];
		int delayMs = Math.Round(step.GetDelay() * 1000.0);

		if (delayMs <= 0)
		{
			OnStepFired();
			return;
		}

		GetGame().GetCallqueue().CallLater(OnStepFired, delayMs, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStepFired()
	{
		if (!m_bRunning || !m_Steps)
			return;

		if (m_iCurrentStep >= m_Steps.Count())
		{
			m_bRunning = false;
			return;
		}

		m_Steps[m_iCurrentStep].Fire();
		m_iCurrentStep++;
		ScheduleCurrentStep();
	}
}
